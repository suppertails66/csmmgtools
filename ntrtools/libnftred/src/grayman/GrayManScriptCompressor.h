#ifndef GRAYMANSCRIPTCOMPRESSOR_H
#define GRAYMANSCRIPTCOMPRESSOR_H


#include "util/TStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TThingyTable.h"
#include "grayman/GrayManScriptReader.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace Nftred {


typedef std::map<int, std::string> GrayManDictionary;

class GrayManScriptCompressor {
public:
  
  GrayManScriptCompressor(
                  GrayManScriptReader::ResultCollection& src__,
                  GrayManDictionary& dict__);
  
  void operator()(int reductionsPerPass = 1,
                  int minSymbols = 2,
                  int maxSymbols = 8);

  const static int dictionaryBase = 0x8100;
protected:
  
  struct CountEntry {
    int useCount;
    int symbolCount;
  };
  
  struct EfficiencyEntry {
    std::string str;
    int symbolCount;
  };
  
  typedef std::map<std::string, CountEntry> StringCountMap;
  typedef std::map<double, EfficiencyEntry> StringEfficiencyMap;

  GrayManScriptReader::ResultCollection& src;
  GrayManDictionary& dict;
//  BlackT::TThingyTable thingy;
//  int lineNum;
//  bool breakTriggered;
  
//  void loadThingy(const BlackT::TThingyTable& thingy__);

  bool isDictionaryable(const GrayManScriptReader::ScriptSymbol symbol) const;
  
  void countDictionaryableStringsOfLen(StringCountMap& dst, int len) const;
  void countDictionaryableStringsOfLen(
    const GrayManScriptReader::ScriptSymbolCollection& src,
    StringCountMap& dst, int len) const;
  
  std::string getDictionaryableString(
    GrayManScriptReader::ScriptSymbolCollection::const_iterator it,
    int len) const;
  
  void addToDictionary(std::string str, int numSymbols);
  
};


}


#endif
