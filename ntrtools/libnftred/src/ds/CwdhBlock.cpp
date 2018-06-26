#include "ds/CwdhBlock.h"
#include "util/TSerialize.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nftred {


CwdhBlock::CwdhBlock()
  : signature { 'C', 'W', 'D', 'H' },
    size(0),
    firstGlyphCode(0),
    lastGlyphCode(0),
    errorCwdhBlock(0) {
  
}
  
void CwdhBlock::read(BlackT::TStream& ifs) {
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  firstGlyphCode
    = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  lastGlyphCode
    = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  errorCwdhBlock
    = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  
  int numGlyphs = (lastGlyphCode - firstGlyphCode) + 1;
    
  glyphEntries.resize(numGlyphs);
  for (int i = 0; i < numGlyphs; i++) {
    glyphEntries[i].bearingX
      = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
    glyphEntries[i].width
      = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
    glyphEntries[i].advanceWidth
      = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  }
}

void CwdhBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();

  ofs.writeRev(signature, sizeof(signature));
  ofs.writeInt(size, 4, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(firstGlyphCode, 2,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(lastGlyphCode, 2,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(errorCwdhBlock, 4,
               EndiannessTypes::little, SignednessTypes::nosign);
               
  for (unsigned int i = 0; i < glyphEntries.size(); i++) {
    ofs.writeInt(glyphEntries[i].bearingX, 1,
                 EndiannessTypes::little, SignednessTypes::nosign);
    ofs.writeInt(glyphEntries[i].width, 1,
                 EndiannessTypes::little, SignednessTypes::nosign);
    ofs.writeInt(glyphEntries[i].advanceWidth, 1,
                 EndiannessTypes::little, SignednessTypes::nosign);
  }
  
  ofs.alignToBoundary(4);
  
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


} 
