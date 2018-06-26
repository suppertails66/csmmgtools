#ifndef FINFBLOCK_H
#define FINFBLOCK_H


#include "ds/NitroHeader.h"
#include "util/TStringConversion.h"
#include "util/TStream.h"
#include "util/TByte.h"

namespace Nftred {


namespace FinfEncodingTypes {
  enum FinfEncodingType {
    utf8 = 0,
    unicode = 1,
    sjis = 2,
    cp1252 = 3
  };
}

class FinfBlock {
public:
  FinfBlock();
  
  void read(BlackT::TStream& ifs, const NitroHeader& header);
  void write(BlackT::TStream& ofs, const NitroHeader& header) const;

  char signature[4];
  int size;
  int a;
  int height;
  int errorCharIndex;
  int defaultBearingX;
  int defaultWidth;
  int defaultAdvanceWidth;
  FinfEncodingTypes::FinfEncodingType encodingType;
  int cglpOffset;
  int cwdhOffset;
  int cmapOffset;
  
  // 1.2 only
  int glyphHeight;
  int glyphWidth;
  int glyphBearingY;
  int glyphBearingX;
  
};


}


#endif
