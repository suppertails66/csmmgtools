#include "ds/NcgrFile.h"

namespace Nftred {


NcgrFile::NcgrFile()
  : hasCposBlock(false) { }
  
void NcgrFile::read(BlackT::TStream& ifs) {
  header.read(ifs);
//  int version = header.version;
  charBlock.read(ifs);
  if (!ifs.nextIsEof()) {
    hasCposBlock = true;
    cposBlock.read(ifs);
  }
}

void NcgrFile::write(BlackT::TStream& ofs) const {
  int startpos = 0;
  
  header.write(ofs);
  
  charBlock.write(ofs);
  if (hasCposBlock) {
    cposBlock.write(ofs);
  }
  
  // write filesize to header
  int endpos = ofs.tell();
  ofs.seek(startpos + 8);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


} 
