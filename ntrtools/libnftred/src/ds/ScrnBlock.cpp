#include "ds/ScrnBlock.h"
#include "util/TSerialize.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nftred {


ScrnBlock::ScrnBlock()
  : signature { 'S', 'C', 'R', 'N' },
    size(0),
    width(0),
    height(0),
    internalSize(0),
    bgType(0) {
  
}
  
void ScrnBlock::read(BlackT::TStream& ifs) {
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readu32le();
  
  width = ifs.readu16le();
  height = ifs.readu16le();
  internalSize = ifs.readu16le();
  bgType = ifs.readu16le();
  
  int dataSize = ifs.readu32le();
//  int dataOffset =
//    ifs.readu32le();
  data.resize(dataSize);
  ifs.read((char*)(data.data()), dataSize);
}

void ScrnBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeu32le(size);
  
  ofs.writeu16le(width);
  ofs.writeu16le(height);
  ofs.writeu16le(internalSize);
  ofs.writeu16le(bgType);
  
  ofs.writeu32le(data.size());
  ofs.write((char*)(data.data()), data.size());
  
  ofs.alignToBoundary(4);
  
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}

void ScrnBlock::saveToEditable(const char* prefix) const {
  
}

void ScrnBlock::loadFromEditable(const char* prefix) {
  
}


} 
