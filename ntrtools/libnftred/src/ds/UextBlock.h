#ifndef UEXTBLOCK_H
#define UEXTBLOCK_H


#include "util/TStream.h"
#include <string>
#include <vector>

namespace Nftred {


class UextBlock {
public:
  UextBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  char signature[4];
  int size;
  int unknown;
protected:
  
};


}


#endif
