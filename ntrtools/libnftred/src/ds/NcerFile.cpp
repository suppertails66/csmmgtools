#include "ds/NcerFile.h"

namespace Nftred {


NcerFile::NcerFile()
  : hasLablBlock(false) { }

void NcerFile::read(BlackT::TStream& ifs) {
  header.read(ifs);
  cebkBlock.read(ifs);
  
  if (!ifs.nextIsEof()) {
    hasLablBlock = true;
    lablBlock.read(ifs);
    hasUextBlock = true;
    uextBlock.read(ifs);
  }
}

void NcerFile::write(BlackT::TStream& ofs) const {
  int startpos = 0;
  
  header.write(ofs);
  cebkBlock.write(ofs);
  
  if (hasLablBlock) {
    lablBlock.write(ofs);
  }
  
  if (hasUextBlock) {
    uextBlock.write(ofs);
  }
  
  // write filesize to header
  int endpos = ofs.tell();
  ofs.seek(startpos + 8);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


}
