#ifndef NITROHEADER_H
#define NITROHEADER_H


#include "util/TStringConversion.h"
#include "util/TStream.h"
#include "util/TByte.h"

namespace Nftred {


class NitroHeader {
public:
  const static int size = 16;

  NitroHeader();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;

  char signature[4];
  int bom;
  int version;
  int filesize;
  int offset;
  int numblocks;
};


}


#endif
