#include "ds/NclrFile.h"

namespace Nftred {


NclrFile::NclrFile() { }

void NclrFile::read(BlackT::TStream& ifs) {
  header.read(ifs);
  plttBlock.read(ifs);
}

void NclrFile::write(BlackT::TStream& ofs) const {
  int startpos = 0;
  
  header.write(ofs);
  plttBlock.write(ofs);
  
  // write filesize to header
  int endpos = ofs.tell();
  ofs.seek(startpos + 8);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


}
