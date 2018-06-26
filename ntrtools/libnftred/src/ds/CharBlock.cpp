#include "ds/CharBlock.h"
#include "util/TSerialize.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nftred {


CharBlock::CharBlock()
  : signature { 'C', 'H', 'A', 'R' },
    size(0),
    height(0),
    width(0),
    bpp(0),
    width2(0),
    height2(0),
    notTiled(false),
    partitioned(false),
    unknown(0) {
  
}
  
void CharBlock::read(BlackT::TStream& ifs) {
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readu32le();
  height = ifs.readu16le();
  width = ifs.readu16le();
  bpp = ifs.readu32le();
  height2 = ifs.readu16le();
  width2 = ifs.readu16le();
  notTiled = (ifs.readu8le() != 0);
  partitioned = (ifs.readu8le() != 0);
  unknown = ifs.readu16le();
  
  int dataSize = ifs.readu32le();
//  int dataOffset =
    ifs.readu32le();
  data.resize(dataSize);
  ifs.read((char*)(data.data()), dataSize);
}

void CharBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeu32le(size);
  ofs.writeu16le(height);
  ofs.writeu16le(width);
  ofs.writeu32le(bpp);
  ofs.writeu16le(height2);
  ofs.writeu16le(width2);
  ofs.writeu8le(notTiled);
  ofs.writeu8le(partitioned);
  ofs.writeu16le(unknown);
  ofs.writeu32le(data.size());
  ofs.writeu32le(0x18); // data offset
  ofs.write((char*)(data.data()), data.size());
  
  ofs.alignToBoundary(4);
  
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


} 
