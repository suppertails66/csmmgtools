#include "ds/LablBlock.h"
#include "util/ByteConversion.h"
#include <iostream>
#include <cstring>

using namespace BlackT;

namespace Nftred {

  
LablBlock::LablBlock()
  : signature { 'L', 'A', 'B', 'L' },
    size(0) { }
  
void LablBlock::read(BlackT::TStream& ifs) {
//  int startpos = ifs.tell();
  
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readu32le();
  
  // this chunk is very ill-conceived:
  // first, we have a table of offsets into the string table. the number of
  // offsets is variable (and independent of the number of cell banks, etc.).
  // after that, we have the string table.
  // unfortunately, we have no way of knowing how many offsets there are,
  // so we don't know where the string table is! it's impossible to actually
  // do anything with this information!
  // 
  // so we have to use some heuristics
  
  std::vector<int> offsets;
  offsets.push_back(ifs.readu32le());
  while (true) {
    int next = ifs.readu32le();
    int prev = offsets[offsets.size() - 1];
    if ((next >= 0x8000)
        || (next <= prev)
        || (next - prev >= 260)) break;
    
    offsets.push_back(next);
  }
  
  // undo the last read
  ifs.seekoff(-4);
  
  for (int i = 0; i < offsets.size(); i++) {
    std::string str;
    char next;
    while ((next = ifs.get())) {
      str += next;
    }
    strings.push_back(str);
  }
  
  
  
//  int remaining = size - (ifs.tell() - startpos);
}

void LablBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeu32le(size);
  
  int indexpos = ofs.tell();
  ofs.seekoff(strings.size() * 4);
  
  std::vector<int> stringPositions;
  stringPositions.resize(strings.size());
  
  int stringsStart = ofs.tell();
  for (int i = 0; i < strings.size(); i++) {
    stringPositions[i] = ofs.tell() - stringsStart;
    ofs.write(strings[i].c_str(), strings[i].size() + 1);
  }
  
  ofs.alignToBoundary(4);
  int endpos = ofs.tell();
  
  ofs.seek(indexpos);
  for (int i = 0; i < stringPositions.size(); i++) {
    ofs.writeu32le(stringPositions[i]);
  }
  
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


}
