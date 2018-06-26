#ifndef CHARBLOCK_H
#define CHARBLOCK_H


#include "util/TStringConversion.h"
#include "util/TStream.h"
#include "util/TByte.h"
#include "util/TArray.h"
#include <vector>

namespace Nftred {


class CharBlock {
public:
  CharBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;

  char signature[4];
  int size;
  // width/height, but may be 0xFFFF
  int height;
  int width;
  int bpp;
  // width/height if above are 0xFFFF
  int width2;
  int height2;
  bool notTiled;
  bool partitioned;
  int unknown;
  BlackT::TArray<BlackT::TByte> data;
  
};


}


#endif
