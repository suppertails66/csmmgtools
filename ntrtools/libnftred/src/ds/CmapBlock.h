#ifndef CMAPBLOCK_H
#define CMAPBLOCK_H


#include "ds/NitroHeader.h"
#include "util/TStringConversion.h"
#include "util/TStream.h"
#include "util/TByte.h"
#include "util/TArray.h"
#include <vector>

namespace Nftred {


struct CmapType2Entry {
  int charCode;
  int glyphNum;
};

class CmapBlock {
public:
  const static int headerSize = 16;

  CmapBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;

  char signature[4];
  int size;
  int firstGlyphCode;
  int lastGlyphCode;
  int cmapType;
  int nextBlockOffset;
  
  // cmap type 0
  int firstCharGlyphNum;
  
  // cmap type 1
  std::vector<int> glyphNums;
  
  // cmap type 2
//  int numGlyphChars;
  std::vector<CmapType2Entry> cmapType2Entries;
  
};


}


#endif
