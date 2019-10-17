#ifndef GRAYMANSCRIPTREADER_H
#define GRAYMANSCRIPTREADER_H


#include "util/TStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TThingyTable.h"
#include <list>
#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace Nftred {


class GrayManScriptReader {
public:

  // TODO: #ENABLEDICTIONARY(), #DISABLEDICTIONARY()

  struct ScriptSymbol {
    enum Type {
      type_literal,
      type_dictionaryEntry,
      type_special
    };
    
    Type type;
//    int key;
    std::string key;
    int keyAsInt;
    
    ScriptSymbol();
    ScriptSymbol(int key__);
    ScriptSymbol(Type type__, int key__);
    ScriptSymbol(std::string key__);
    ScriptSymbol(Type type__, std::string key__);
    
    void setKey(int key__);
    
    int size() const;
  };
  
  typedef std::list<ScriptSymbol> ScriptSymbolCollection;

  struct ResultString {
    std::string str;
    ScriptSymbolCollection symbols;
    int srcOffset;
    int srcSize;
//    int srcSlot;
    std::string dstFile;
    int srcLine;
    
    std::string getString() const;
  };
  typedef std::vector<ResultString> ResultCollection;

  GrayManScriptReader(BlackT::TStream& src__,
//                  BlackT::TStream& dst__,
//                  NesRom& dst__,
                  ResultCollection& dst__,
                  const BlackT::TThingyTable& thingy__);
  
  bool operator()();
protected:

  BlackT::TStream& src;
  ResultCollection& dst;
  BlackT::TThingyTable thingy;
  int lineNum;
  bool breakTriggered;
  
  BlackT::TBufStream currentScriptBuffer;
  ScriptSymbolCollection currentScriptSymbols;
  int currentScriptSrcOffset;
  int currentScriptSrcSize;
//  int currentScriptSrcSlot;
  std::string currentScriptDstFile;
  
  void outputNextSymbol(BlackT::TStream& ifs);
  
//  bool checkSymbol(BlackT::TStream& ifs, std::string& symbol);
  
  void flushActiveScript();
  void resetScriptBuffer();
  
  void processDirective(BlackT::TStream& ifs);
  void processLoadTable(BlackT::TStream& ifs);
  void processStartMsg(BlackT::TStream& ifs);
  void processEndMsg(BlackT::TStream& ifs);
  void processIncBin(BlackT::TStream& ifs);
  void processBreak(BlackT::TStream& ifs);
  void processSetDstFile(BlackT::TStream& ifs);
  
  void loadThingy(const BlackT::TThingyTable& thingy__);
  
};


}


#endif
