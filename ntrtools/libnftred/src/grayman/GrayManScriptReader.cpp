#include "grayman/GrayManScriptReader.h"
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


const static int scriptBufferCapacity = 0x10000;

GrayManScriptReader::ScriptSymbol::ScriptSymbol()
  : type(ScriptSymbol::type_literal),
    keyAsInt(-1) { };

GrayManScriptReader::ScriptSymbol::ScriptSymbol(int key__)
  : type(ScriptSymbol::type_literal),
    keyAsInt(key__) {
  setKey(key__);
}

GrayManScriptReader::ScriptSymbol::ScriptSymbol(Type type__, int key__)
  : type(type__),
    keyAsInt(key__) {
  setKey(key__);
}

GrayManScriptReader::ScriptSymbol::ScriptSymbol(std::string key__)
  : type(ScriptSymbol::type_literal),
    key(key__) { };

GrayManScriptReader::ScriptSymbol::ScriptSymbol(Type type__, std::string key__)
  : type(type__),
    key(key__) { };

void GrayManScriptReader::ScriptSymbol::setKey(int key__) {
  keyAsInt = key__;
  key = "";
  for (int i = 0; i < sizeof(int); i++) {
//    key = TStringConversion::intToString((key__ & 0xFF),
//            TStringConversion::baseHex).substr(2, std::string::npos)
//          + key;
    key = (char)(key__ & 0xFF) + key;
    key__ >>= 8;
    if (key__ == 0) break;
  }
}

int GrayManScriptReader::ScriptSymbol::size() const {
  return key.size();
}

std::string GrayManScriptReader::ResultString::getString() const {
  std::string result;
  for (ScriptSymbolCollection::const_iterator it = symbols.cbegin();
       it != symbols.cend();
       ++it) {
    result += it->key;
  }
  return result;
}

GrayManScriptReader::GrayManScriptReader(
                  BlackT::TStream& src__,
                  ResultCollection& dst__,
                  const BlackT::TThingyTable& thingy__)
  : src(src__),
    dst(dst__),
    thingy(thingy__),
    lineNum(0),
    breakTriggered(false),
    currentScriptBuffer(scriptBufferCapacity) {
  loadThingy(thingy__);
//  spaceOfs.open((outprefix + "msg_space.txt").c_str());
//  indexOfs.open((outprefix + "msg_index.txt").c_str());
  resetScriptBuffer();
}

bool GrayManScriptReader::operator()() {
  try {
    while (!src.eof()) {
      std::string line;
      src.getLine(line);
      ++lineNum;
      
//      std::cerr << "ScriptReader: line " << lineNum << std::endl;
      if (line.size() <= 0) continue;
      
      // discard lines containing only ASCII spaces and tabs
  //    bool onlySpace = true;
  //    for (int i = 0; i < line.size(); i++) {
  //      if ((line[i] != ' ')
  //          && (line[i] != '\t')) {
  //        onlySpace = false;
  //        break;
  //      }
  //    }
  //    if (onlySpace) continue;
      
      TBufStream ifs(line.size());
      ifs.write(line.c_str(), line.size());
      ifs.seek(0);
      
      // check for special stuff
      if (ifs.peek() == '#') {
        // directives
        ifs.get();
        if (!islower(ifs.peek())) {
          processDirective(ifs);
          
          if (breakTriggered) {
            breakTriggered = false;
            return false;
          }
          
          continue;
        }
        else ifs.unget();
      }
      
      while (!ifs.eof()) {
        // check for comments
        if ((ifs.remaining() >= 2)
            && (ifs.peek() == '/')) {
          ifs.get();
          if (ifs.peek() == '/') break;
          else ifs.unget();
        }
        
        outputNextSymbol(ifs);
      }
    }
    
    if (currentScriptBuffer.size() > 0) {
      flushActiveScript();
    }
    
    return true;
  }
  catch (TGenericException& e) {
    std::cerr << "Exception on script line " << lineNum << ":" << std::endl;
    std::cerr << "  " << e.problem() << std::endl;
    throw e;
  }
}
  
void GrayManScriptReader::loadThingy(const BlackT::TThingyTable& thingy__) {
  thingy = thingy__;
}
  
void GrayManScriptReader::outputNextSymbol(TStream& ifs) {
  // literal value
  if ((ifs.remaining() >= 5)
      && (ifs.peek() == '<')) {
    int pos = ifs.tell();
    
    ifs.get();
    if (ifs.peek() == '$') {
      ifs.get();
      std::string valuestr = "0x";
      valuestr += ifs.get();
      valuestr += ifs.get();
      
      if (ifs.peek() == '>') {
        ifs.get();
        int value = TStringConversion::stringToInt(valuestr);
        
//        dst.writeu8(value);
        currentScriptBuffer.writeu8(value);
        currentScriptSymbols.push_back(
          ScriptSymbol(ScriptSymbol::type_special, value));

        return;
      }
    }
    
    // not a literal value
    ifs.seek(pos);
  }
  
  TThingyTable::MatchResult result;
  result = thingy.matchTableEntry(ifs);
/*  if ((ifs.peek() != '[')
      && ((unsigned int)(unsigned char)ifs.peek() < 0x80)) {
    result.id = ifs.readu8();
  }
  else if ((unsigned int)(unsigned char)ifs.peek() >= 0x80) {
    result.id = ifs.readu16be();
  }
  else {
    result = thingy.matchTableEntry(ifs);
  } */
  
  // the following require special handling:
  // * sequences enclosed by < >, e.g. <W7>. these are delay commands, where
  //   the value (which may contain any number of digits) specifies the length
  //   of the delay.
  // * #XX, e.g. #cy. these are color change commands. XX is always 2 digits.
  // handling is not robust, e.g. no check for failure to match an ID, so
  // don't do something stupid
  if (((char)result.id == '#')
      && (ifs.remaining() >= 2)) {
    std::string resultString;
    resultString += (char)result.id;
    resultString += (char)thingy.matchTableEntry(ifs).id;
    resultString += (char)thingy.matchTableEntry(ifs).id;
    currentScriptBuffer.write(resultString.c_str(), resultString.size());
    currentScriptSymbols.push_back(
      ScriptSymbol(ScriptSymbol::type_special, resultString));
    return;
  }
  // do not dictionary "%s" or similar format string sequences
  else if (((char)result.id == '%')
      && (ifs.remaining() >= 1)) {
    std::string resultString;
    resultString += (char)result.id;
    resultString += (char)thingy.matchTableEntry(ifs).id;
    currentScriptBuffer.write(resultString.c_str(), resultString.size());
    currentScriptSymbols.push_back(
      ScriptSymbol(ScriptSymbol::type_special, resultString));
    return;
  }
  else if (((char)result.id == '<')) {
    int pos = ifs.tell();
    
    std::string resultString;
    resultString += (char)result.id;
    
    while (!ifs.eof()) {
      int nextId = (char)thingy.matchTableEntry(ifs).id;
      if (nextId == -1) break;
      
      resultString += (char)nextId;
      if (nextId == '>') {
        currentScriptBuffer.write(resultString.c_str(), resultString.size());
        currentScriptSymbols.push_back(
          ScriptSymbol(ScriptSymbol::type_special, resultString));
        return;
      }
    }
    
    // failure
    ifs.clear();
    ifs.seek(pos);
  }
  
  if (result.id != -1) {
//    std::cerr << std::dec << lineNum << " " << std::hex << result.id << " " << result.size << std::endl;
  
    int symbolSize;
    if (result.id <= 0xFF) symbolSize = 1;
    else if (result.id <= 0xFFFF) symbolSize = 2;
    else if (result.id <= 0xFFFFFF) symbolSize = 3;
    else symbolSize = 4;
    
    currentScriptBuffer.writeInt(result.id, symbolSize,
      EndiannessTypes::big, SignednessTypes::nosign);
      
    if ((result.id == GrayManConsts::code_br)
        || (result.id == GrayManConsts::code_wait)
        || (result.id == GrayManConsts::code_end)) {
      
/*      if ((currentScriptSymbols.size() == 0)
          && (currentScriptSrcSize > 3)
          && (result.id == GrayManConsts::code_wait)) {
//        currentScriptSymbols.push_back(
//          ScriptSymbol(ScriptSymbol::type_literal, 0x8260));
        currentScriptSymbols.push_back(
          ScriptSymbol(ScriptSymbol::type_literal, 'A'));
      } */
      
      currentScriptSymbols.push_back(
        ScriptSymbol(ScriptSymbol::type_special, result.id));
    }
    else {
      currentScriptSymbols.push_back(ScriptSymbol(result.id));
    }
    
    return;
  }
  
  std::string remainder;
  ifs.getLine(remainder);
  
  // if we reached end of file, this is not an error: we're done
  if (ifs.eof()) return;
  
  throw TGenericException(T_SRCANDLINE,
                          "GrayManScriptReader::outputNextSymbol()",
                          "Line "
                            + TStringConversion::intToString(lineNum)
                            + ":\n  Couldn't match symbol at: '"
                            + remainder
                            + "'");
}
  
void GrayManScriptReader::flushActiveScript() {
  // write terminator
//  currentScriptBuffer.put(0x00);

  int outputSize = currentScriptBuffer.size();
  
  ResultString result;
  currentScriptBuffer.seek(0);
  while (!currentScriptBuffer.eof()) {
    result.str += currentScriptBuffer.get();
  }
  
  // TEMP: insert "A" into blank messages
  if ((currentScriptSymbols.size() == 0)
      || ((currentScriptSrcSize > 3)
          && (currentScriptSymbols.size() == 1)
          && (currentScriptSymbols.front().key.compare("\x81\xA5") == 0))) {
    currentScriptSymbols.insert(currentScriptSymbols.begin(),
      ScriptSymbol(ScriptSymbol::type_literal, 'A'));
  }
  
  result.symbols = currentScriptSymbols;
  result.srcOffset = currentScriptSrcOffset;
  result.srcSize = currentScriptSrcSize;
//  result.srcSlot = currentScriptSrcSlot;
  result.dstFile = currentScriptDstFile;
  result.srcLine = lineNum;
  
  // TEMP: skip lines that are blank or consist only of a [wait]
/*  if ((currentScriptSymbols.size() == 0)
      || ((currentScriptSymbols.size() == 1)
          && (currentScriptSymbols.front().key.compare("\x81\xA5") == 0))) {
    
  }
  else {
    dst.push_back(result);
  } */
  
  dst.push_back(result);
  
  // clear script buffer
  resetScriptBuffer();
}

void GrayManScriptReader::resetScriptBuffer() {
  currentScriptBuffer = TBufStream(scriptBufferCapacity);
  currentScriptSymbols.clear();
  currentScriptSrcOffset = -1;
  currentScriptSrcSize = -1;
//  currentScriptSrcSlot = -1;
  
  // this carries over to succeeding files!
//  currentScriptDstFile = "";
//  result.srcLine = 1;
}
  
/*bool GrayManScriptReader::checkSymbol(BlackT::TStream& ifs, std::string& symbol) {
  if (symbol.size() > ifs.remaining()) return false;
  
  int startpos = ifs.tell();
  for (int i = 0; i < symbol.size(); i++) {
    if (symbol[i] != ifs.get()) {
      ifs.seek(startpos);
      return false;
    }
  }
  
  return true;
} */

void GrayManScriptReader::processDirective(BlackT::TStream& ifs) {
  TParse::skipSpace(ifs);
  
  std::string name = TParse::matchName(ifs);
  TParse::matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
  if (name.compare("LOADTABLE") == 0) {
    processLoadTable(ifs);
  }
  else if (name.compare("STARTMSG") == 0) {
    processStartMsg(ifs);
  }
  else if (name.compare("ENDMSG") == 0) {
    processEndMsg(ifs);
  }
  else if (name.compare("INCBIN") == 0) {
    processIncBin(ifs);
  }
  else if (name.compare("BREAK") == 0) {
    processBreak(ifs);
  }
  else if (name.compare("SETDSTFILE") == 0) {
    processSetDstFile(ifs);
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "GrayManScriptReader::processDirective()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unknown directive: "
                              + name);
  }
  
  TParse::matchChar(ifs, ')');
}

void GrayManScriptReader::processLoadTable(BlackT::TStream& ifs) {
  std::string tableName = TParse::matchString(ifs);
  TThingyTable table(tableName);
  loadThingy(table);
}

void GrayManScriptReader::processStartMsg(BlackT::TStream& ifs) {
  currentScriptSrcOffset = TParse::matchInt(ifs);
  TParse::matchChar(ifs, ',');
  currentScriptSrcSize = TParse::matchInt(ifs);
//  TParse::matchChar(ifs, ',');
//  currentScriptSrcSlot = TParse::matchInt(ifs);
//  TParse::matchChar(ifs, ',');
//  currentScriptDstFile = TParse::matchString(ifs);
}

void GrayManScriptReader::processEndMsg(BlackT::TStream& ifs) {
  flushActiveScript();
}

void GrayManScriptReader::processIncBin(BlackT::TStream& ifs) {
  std::string filename = TParse::matchString(ifs);
  TBufStream src(1);
  src.open(filename.c_str());
  currentScriptBuffer.writeFrom(src, src.size());
}

void GrayManScriptReader::processBreak(BlackT::TStream& ifs) {
  breakTriggered = true;
}

void GrayManScriptReader::processSetDstFile(BlackT::TStream& ifs) {
  currentScriptDstFile = TParse::matchString(ifs);
}


}
