#include "ds/FinfBlock.h"
#include "util/TSerialize.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nftred {


FinfBlock::FinfBlock()
  : signature { 'F', 'I', 'N', 'F' },
    size(0),
    a(0),
    height(0),
    errorCharIndex(0),
    defaultBearingX(0),
    defaultWidth(0),
    defaultAdvanceWidth(0),
    encodingType(FinfEncodingTypes::utf8),
    cglpOffset(0),
    cwdhOffset(0),
    cmapOffset(0),
    glyphHeight(0),
    glyphWidth(0),
    glyphBearingY(0),
    glyphBearingX(0) {
  
}
  
void FinfBlock::read(BlackT::TStream& ifs, const NitroHeader& header) {
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  a = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  height = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  errorCharIndex
    = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  defaultBearingX
    = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  defaultWidth
    = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  defaultAdvanceWidth
    = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  encodingType
    = static_cast<FinfEncodingTypes::FinfEncodingType>
        (ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign));
  cglpOffset
    = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  cwdhOffset
    = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  cmapOffset
    = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  
  if (header.version >= 0x0102) {
    glyphHeight
      = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
    glyphWidth
      = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
    glyphBearingY
      = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
    glyphBearingX
      = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  }
}

void FinfBlock::write(BlackT::TStream& ofs, const NitroHeader& header) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeInt(size, 4, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(a, 1, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(height, 1, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(errorCharIndex, 2,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(defaultBearingX, 1,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(defaultWidth, 1,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(defaultAdvanceWidth, 1,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(static_cast<int>(encodingType), 1,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(cglpOffset, 4,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(cwdhOffset, 4,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(cmapOffset, 4,
               EndiannessTypes::little, SignednessTypes::nosign);
  
  if (header.version >= 0x0102) {
    ofs.writeInt(glyphHeight, 1,
                 EndiannessTypes::little, SignednessTypes::nosign);
    ofs.writeInt(glyphWidth, 1,
                 EndiannessTypes::little, SignednessTypes::nosign);
    ofs.writeInt(glyphBearingY, 1,
                 EndiannessTypes::little, SignednessTypes::nosign);
    ofs.writeInt(glyphBearingX, 1,
                 EndiannessTypes::little, SignednessTypes::nosign);
  }
  
  ofs.alignToBoundary(4);
  
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


} 
