#include "ds/CmapBlock.h"
#include "util/TSerialize.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nftred {


CmapBlock::CmapBlock()
  : signature { 'C', 'M', 'A', 'P' },
    size(0),
    firstGlyphCode(0),
    lastGlyphCode(0),
    cmapType(0),
    nextBlockOffset(0),
    firstCharGlyphNum(0) {
  
}
  
void CmapBlock::read(BlackT::TStream& ifs) {
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  firstGlyphCode
    = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  lastGlyphCode
    = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  cmapType
    = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  nextBlockOffset
    = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  
  int numGlyphs = (lastGlyphCode - firstGlyphCode) + 1;
  
  switch (cmapType) {
  case 0:
    firstCharGlyphNum
      = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
    break;
  case 1:
    glyphNums.resize(numGlyphs);
    for (unsigned int i = 0; i < glyphNums.size(); i++) {
      glyphNums[i]
        = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
    }
    break;
  case 2:
//    numGlyphChars
//      = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
    {
      int numGlyphChars
        = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
      cmapType2Entries.resize(numGlyphChars);
      for (unsigned int i = 0; i < cmapType2Entries.size(); i++) {
        cmapType2Entries[i].charCode
          = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
        cmapType2Entries[i].glyphNum
          = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
      }
    }
    break;
  default:
    throw TGenericException(T_SRCANDLINE,
                            "CmapBlock::read(BlackT::TStream&,"
                                             "const NitroHeader&)",
                            std::string("Unknown CMAP type: ")
                              + TStringConversion::toString(cmapType));
    break;
  }
}

void CmapBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeInt(size, 4, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(firstGlyphCode, 2,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(lastGlyphCode, 2,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(cmapType, 4,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(nextBlockOffset, 4,
               EndiannessTypes::little, SignednessTypes::nosign);
  
  switch (cmapType) {
  case 0:
    ofs.writeInt(firstCharGlyphNum, 2,
                 EndiannessTypes::little, SignednessTypes::nosign);
    break;
  case 1:
    for (unsigned int i = 0; i < glyphNums.size(); i++) {
      ofs.writeInt(glyphNums[i], 2,
                   EndiannessTypes::little, SignednessTypes::nosign);
    }
    break;
  case 2:
//    ofs.writeInt(numGlyphChars, 2,
//                 EndiannessTypes::little, SignednessTypes::nosign);
    ofs.writeInt(cmapType2Entries.size(), 2,
                 EndiannessTypes::little, SignednessTypes::nosign);
    for (unsigned int i = 0; i < cmapType2Entries.size(); i++) {
      ofs.writeInt(cmapType2Entries[i].charCode, 2,
                   EndiannessTypes::little, SignednessTypes::nosign);
      ofs.writeInt(cmapType2Entries[i].glyphNum, 2,
                   EndiannessTypes::little, SignednessTypes::nosign);
    }
    break;
  default:
    throw TGenericException(T_SRCANDLINE,
                            "CmapBlock::write(BlackT::TStream&,"
                                             "const NitroHeader&)",
                            std::string("Unknown CMAP type: ")
                              + TStringConversion::toString(cmapType));
    break;
  }
  
  ofs.alignToBoundary(4);
  
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


} 
