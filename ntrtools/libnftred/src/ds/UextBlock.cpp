#include "ds/UextBlock.h"
#include "util/ByteConversion.h"
#include <iostream>
#include <cstring>

using namespace BlackT;

namespace Nftred {

  
UextBlock::UextBlock()
  : signature { 'U', 'E', 'X', 'T' },
    size(0) { }
  
void UextBlock::read(BlackT::TStream& ifs) {
//  int startpos = ifs.tell();
  
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readu32le();
  unknown = ifs.readu32le();
  
//  int remaining = size - (ifs.tell() - startpos);
}

void UextBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeu32le(size);
  ofs.writeu32le(unknown);
  
  ofs.alignToBoundary(4);
  
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


}
