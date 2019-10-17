#include "grayman/GrayManScriptCompressor.h"
#include "grayman/GrayManConsts.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "util/TParse.h"
#include "exception/TGenericException.h"
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>

using namespace BlackT;

namespace Nftred {


GrayManScriptCompressor::GrayManScriptCompressor(
                  GrayManScriptReader::ResultCollection& src__,
                  GrayManDictionary& dict__)
  : src(src__),
    dict(dict__) {
  
}

void GrayManScriptCompressor::operator()(
                  int reductionsPerPass,
                  int minSymbols,
                  int maxSymbols) {
  
//  for (int i = 0; i < numPasses; i++) {
  int count = 0;
  while (true) {
//    std::cerr << "Pass " << i << ":" << std::endl;
    std::cerr << "Dictionary pass " << count++ << ":" << std::endl;
  
    StringCountMap stringCounts;
    for (int j = minSymbols; j <= maxSymbols; j++) {
      countDictionaryableStringsOfLen(stringCounts, j);
    }
    
    std::cerr << "  Strings remaining: " << stringCounts.size() << std::endl;
    
    // if we found no dictionaryable strings, we're done
    if (stringCounts.size() == 0) break;
    
    // generate efficiency map
    StringEfficiencyMap stringEfficiencies;
    for (StringCountMap::iterator it = stringCounts.begin();
         it != stringCounts.end();
         ++it) {
      // the effective size for calculating the efficiency is two bytes
      // less than the actual size, since we have to insert a two-byte
      // sequence to call the dictionary entry
      double adjustedSize = it->first.size() - 2;
      if (adjustedSize <= 0) continue;
      
      // lower = better
      double efficiency
        = (double)1 / (adjustedSize * (double)it->second.useCount);
      
      EfficiencyEntry entry;
      entry.str = it->first;
      entry.symbolCount = it->second.symbolCount;
      stringEfficiencies[efficiency] = entry;
    }
    
    StringEfficiencyMap::iterator it = stringEfficiencies.begin();
//    bool done = false;
    for (int j = 0; j < reductionsPerPass; j++) {
      if (it == stringEfficiencies.end()) {
//        done = true;
        break;
      }
      
      addToDictionary(it->second.str, it->second.symbolCount);
      ++it;
    }
    
//    if (done) break;
    
//    for (StringCountMap::iterator it = stringCounts.begin();
//         it != stringCounts.end();
//         ++it) {
//      std::cerr << it->first << " " << it->second << std::endl;
//    }
    
//    for (StringEfficiencyMap::iterator it = stringEfficiencies.begin();
//         it != stringEfficiencies.end();
//         ++it) {
//      std::cerr << it->first << " "
//        << it->second.str << " "
//        << it->second.symbolCount << std::endl;
//    }
  }
}

bool GrayManScriptCompressor
  ::isDictionaryable(const GrayManScriptReader::ScriptSymbol symbol) const {
  if ((symbol.type != GrayManScriptReader::ScriptSymbol::type_literal)
      && (symbol.type 
            != GrayManScriptReader::ScriptSymbol::type_dictionaryEntry))
    return false;
  
  // these are now flagged as special
//  if ((symbol.keyAsInt == GrayManConsts::code_wait)
//      || (symbol.keyAsInt == GrayManConsts::code_br)
//      || (symbol.keyAsInt == GrayManConsts::code_end)) {
//    return false;
//  }
  
  return true;
}

void GrayManScriptCompressor
  ::countDictionaryableStringsOfLen(StringCountMap& dst, int len) const {
  for (GrayManScriptReader::ResultCollection::const_iterator scriptIt
        = src.cbegin();
       scriptIt != src.cend();
       ++scriptIt) {
    // consider only those strings which have not yet been compressed to or
    // beyond their target size
    if ((scriptIt->getString().size() + 1) > scriptIt->srcSize) {
      countDictionaryableStringsOfLen(scriptIt->symbols, dst, len);
    }
  }
}

void GrayManScriptCompressor::countDictionaryableStringsOfLen(
    const GrayManScriptReader::ScriptSymbolCollection& src,
    StringCountMap& dst, int len) const {
  // check all substrings up to the maximum position where a substring of the
  // specified length can actually occur
  int pos = 0;
  int endPos = (src.size() - len) + 1;
  GrayManScriptReader::ScriptSymbolCollection::const_iterator baseIt
    = src.cbegin();
  while (pos < endPos) {
    GrayManScriptReader::ScriptSymbolCollection::const_iterator it
      = baseIt++;
    ++pos;
    
/*    bool matched = true;
    std::string content;
    for (int i = 0; i < len; i++) {
      if (!isDictionaryable(*it)) {
        matched = false;
        break;
      }
      
      content += it->key;
      ++it;
//      ++pos;
    } */
    
    std::string content = getDictionaryableString(it, len);
    
//    if (matched) {
    
    if (content.size() > 0) {
//      baseIt = it;
//      pos += len;
      StringCountMap::iterator findIt = dst.find(content);
      
      if (findIt == dst.end()) {
        CountEntry entry;
        entry.useCount = 1;
        entry.symbolCount = len;
        dst[content] = entry;
      }
      else ++(findIt->second.useCount);
    }
    else {
      // baseIt already points to the next position in the input
    }
  }
}

std::string GrayManScriptCompressor::getDictionaryableString(
    GrayManScriptReader::ScriptSymbolCollection::const_iterator it,
    int len) const {
  std::string content;
  for (int i = 0; i < len; i++) {
    if (!isDictionaryable(*it)) return "";
    
    content += it->key;
    ++it;
//      ++pos;
  }
  return content;
}

void GrayManScriptCompressor
  ::addToDictionary(std::string str, int numSymbols) {
  
  int newDictIndex = dict.size() + dictionaryBase;
  
  // we must not conflict with the special "wait" code, which is mapped
  // to a plain SJIS symbol (81A5)
  // or any of the other special symbols
  while ((newDictIndex == GrayManConsts::code_wait)
         || (newDictIndex == GrayManConsts::code_spaceFullWidth)
         || (newDictIndex == GrayManConsts::code_unk1)
         || (newDictIndex == GrayManConsts::code_unk2)
         || (newDictIndex == GrayManConsts::code_unk3)
         || (newDictIndex == GrayManConsts::code_unk4)
         || ((newDictIndex >= GrayManConsts::code_digitsLow)
             && (newDictIndex <= GrayManConsts::code_digitsHigh))
         // ban sequences with a low byte of 00, since this breaks
         // string length calculations
         || ((newDictIndex & 0x00FF) == 0)) {
    dict[newDictIndex] = "";
    ++newDictIndex;
  }
  
  if (newDictIndex > 0xFFFF) {
    throw TGenericException(T_SRCANDLINE,
                            "GrayManScriptCompressor::addToDictionary()",
                            "Dictionary full"
                            " -- lower the compression settings");
  }
  
  dict[newDictIndex] = str;
  
  GrayManScriptReader::ScriptSymbol dictSymbol(
    GrayManScriptReader::ScriptSymbol::type_dictionaryEntry,
    newDictIndex);
        
  for (GrayManScriptReader::ResultCollection::iterator scriptIt
        = src.begin();
       scriptIt != src.end();
       ++scriptIt) {
    GrayManScriptReader::ScriptSymbolCollection& src
      = scriptIt->symbols;
    
    int pos = 0;
    int endPos = (src.size() - numSymbols) + 1;
    GrayManScriptReader::ScriptSymbolCollection::iterator baseIt
      = src.begin();
    while (pos < endPos) {
      GrayManScriptReader::ScriptSymbolCollection::iterator it
        = baseIt++;
      ++pos;
      
      std::string content = getDictionaryableString(it, numSymbols);
      
      if ((content.size() > 0)
          && (content.compare(str) == 0)) {
//        std::cerr << content << std::endl;

        // get pointer to end of sequence being added to dictionary
        GrayManScriptReader::ScriptSymbolCollection::iterator endIt
          = it;
        for (int i = 0; i < numSymbols; i++) {
          ++endIt;
        }
        
        // this is where we insert the new symbol after the deletion
//        GrayManScriptReader::ScriptSymbolCollection::iterator insertIt
//          = endIt;
//        ++insertIt;
        
        // erase old symbols
        GrayManScriptReader::ScriptSymbolCollection::iterator insertIt
          = scriptIt->symbols.erase(it, endIt);
        
        // insert new dictionary symbol
        scriptIt->symbols.insert(insertIt, dictSymbol);
        
        // update
        endPos -= (numSymbols - 1);
        baseIt = insertIt;
      }
    }
  }
}


}
