#ifndef LABLBLOCK_H
#define LABLBLOCK_H


#include "util/TStream.h"
#include <string>
#include <vector>

namespace Nftred {


class LablBlock {
public:
  LablBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  char signature[4];
  int size;
  std::vector<std::string> strings;
protected:
  
};


}


#endif
