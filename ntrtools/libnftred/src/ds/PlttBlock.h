#ifndef PLTTBLOCK_H
#define PLTTBLOCK_H


#include "util/TStream.h"
#include "ds/NitroPalette.h"
#include <string>
#include <vector>

namespace Nftred {


class PlttBlock {
public:
  PlttBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  char signature[4];
  int size;
  int bpp;
  int unknown1;
  int unknown2;
  int unknown3;
  std::vector<NitroPalette> palettes;
protected:
  
};


}


#endif
