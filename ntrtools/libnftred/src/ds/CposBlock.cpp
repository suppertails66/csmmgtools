#include "ds/CposBlock.h"
#include "util/TSerialize.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nftred {


CposBlock::CposBlock()
  : signature { 'C', 'P', 'O', 'S' },
    size(0),
    zero(0),
    charSize(0),
    charCount(0) {
  
}
  
void CposBlock::read(BlackT::TStream& ifs) {
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readu32le();
  zero = ifs.readu32le();
  charSize = ifs.readu16le();
  charCount = ifs.readu16le();
}

void CposBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeu32le(size);
  ofs.writeu32le(zero);
  ofs.writeu16le(charSize);
  ofs.writeu16le(charCount);
  
  ofs.alignToBoundary(4);
  
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


} 
