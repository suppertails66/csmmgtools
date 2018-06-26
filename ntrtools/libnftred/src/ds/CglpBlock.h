#ifndef CGLPBLOCK_H
#define CGLPBLOCK_H


#include "ds/NitroHeader.h"
#include "util/TStringConversion.h"
#include "util/TStream.h"
#include "util/TByte.h"
#include "util/TArray.h"
#include <vector>

namespace Nftred {


class CglpBlock {
public:
  const static int headerSize = 16;
  
  typedef BlackT::TArray< BlackT::TArray<BlackT::TByte> >
    GlyphDataCollection;

  CglpBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;

  char signature[4];
  int size;
  // actual width/height of stored image data
  int width;
  int height;
  // size in bytes of each glyph's data
  int glyphSize;
  // dunno
  int glyphMetricHeight;
  int glyphMetricWidth;
  int bpp;
  int rotation;
  // data format: top to bottom, right to left, low bits to high bits,
  // wrap to next row at end of previous
  GlyphDataCollection glyphData;
  
};


}


#endif
