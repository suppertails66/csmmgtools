#ifndef CWDHBLOCK_H
#define CWDHBLOCK_H


#include "ds/NitroHeader.h"
#include "util/TStringConversion.h"
#include "util/TStream.h"
#include "util/TByte.h"
#include "util/TArray.h"
#include <vector>

namespace Nftred {


struct CwdhGlyphEntry {
  const static int size = 3;

  // amount by which to offset image data
  int bearingX;
  // width of "character" within the image data
  // e.g. if the raw pixel data is 12 pixels wide and this value is 4, the
  // 4 columns on the left contain pixel data
  int width;
  // amount by which to advance printing position after printing the character
  int advanceWidth;
};

class CwdhBlock {
public:
  const static int headerSize = 16;
  
  typedef BlackT::TArray< CwdhGlyphEntry >
    CwdhGlyphEntryCollection;

  CwdhBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;

  char signature[4];
  int size;
  int firstGlyphCode;
  int lastGlyphCode;
  int errorCwdhBlock;
  CwdhGlyphEntryCollection glyphEntries;
  
};


}


#endif
