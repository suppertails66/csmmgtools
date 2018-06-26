#ifndef CPOSBLOCK_H
#define CPOSBLOCK_H


#include "util/TStringConversion.h"
#include "util/TStream.h"
#include "util/TByte.h"
#include "util/TArray.h"
#include <vector>

namespace Nftred {


class CposBlock {
public:
  CposBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;

  char signature[4];
  int size;
  int zero;
  int charSize;
  int charCount;
  
};


}


#endif
