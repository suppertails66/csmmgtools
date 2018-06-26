#include "ds/CglpBlock.h"
#include "util/TSerialize.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nftred {


CglpBlock::CglpBlock()
  : signature { 'C', 'G', 'L', 'P' },
    size(0),
    width(0),
    height(0),
    glyphSize(0),
    glyphMetricHeight(0),
    glyphMetricWidth(0),
    bpp(0),
    rotation(0) {
  
}
  
void CglpBlock::read(BlackT::TStream& ifs) {
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  width = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  height = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  glyphSize
    = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  glyphMetricHeight
    = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  glyphMetricWidth
    = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  bpp
    = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  rotation
    = ifs.readInt(1, EndiannessTypes::little, SignednessTypes::nosign);
  
  int numGlyphs = (size - headerSize) / glyphSize;
    
  glyphData.resize(numGlyphs);
  for (int i = 0; i < numGlyphs; i++) {
    glyphData[i].resize(glyphSize);
    ifs.read((char*)(glyphData[i].data()), glyphSize);
  }
}

void CglpBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeInt(size, 4, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(width, 1, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(height, 1, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(glyphSize, 2,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(glyphMetricHeight, 1,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(glyphMetricWidth, 1,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(bpp, 1,
               EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(rotation, 1,
               EndiannessTypes::little, SignednessTypes::nosign);
               
  for (unsigned int i = 0; i < glyphData.size(); i++) {
    ofs.write((char*)(glyphData[i].data()), glyphSize);
  }
  
  ofs.alignToBoundary(4);
  
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


} 
