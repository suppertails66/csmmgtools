#ifndef BVMDATA_H
#define BVMDATA_H


#include "util/TByte.h"
#include <vector>
#include <string>

namespace Nftred {


const static BlackT::TByte shortStringRefOpcode = 0x5A;
const static BlackT::TByte longStringRefOpcode = 0x9A;
const static BlackT::TByte centeringStartOpcode = 0x55;
const static BlackT::TByte centeringThirdByte = 0xA9;

const static int minCenteringXValue = 0x1;
const static int maxCenteringXValue = 0x14;
const static int minCenteringYValue = 0x1;
const static int maxCenteringYValue = 0x3;

struct CenteringCommand {
  bool hasX;
  int offsetOfX;
  int numChars;
  
  bool hasY;
  int offsetOfY;
  int numLines;
};

struct BvmStringEntry {
  std::string filename;
  int number;
  int offset;
  std::string japanese;
  std::string english;
  std::vector<CenteringCommand> centeringCommands;
  std::vector<int> offsetsByte;
  std::vector<int> offsetsWord;
  int width;
  int height;
  int lineheight;
  
  void output(std::ostream& csvofs) const;
};

void escapeString(const std::string& src, std::string& dst);

void unescapeString(const std::string& src, std::string& dst);


}


#endif
