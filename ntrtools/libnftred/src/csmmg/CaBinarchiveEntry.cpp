#include "csmmg/CaBinarchiveEntry.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include <cstring>
#include <iostream>
#include <string>

using namespace std;
using namespace BlackT;

namespace Nftred {


CaBinarchiveEntry::CaBinarchiveEntry() { }
  
void CaBinarchiveEntry::readRaw(BlackT::TStream& ifs) {
//  cout << hex << ifs.tell() << endl;
  int chunkstart = ifs.tell();

  unknown1 = ifs.readu32le();
  int subfileTableOffset = ifs.readu32le();
  int dataOffset = ifs.readu32le();
  int filenameChunkOffset = ifs.readu32le();
  unknown2 = ifs.readu32le();
  unknown3 = ifs.readu32le();
  
//  cout << hex << ifs.tell() << endl;
  int numentriesRaw = ifs.readu32le();
  int numentries = numentriesRaw / 2;
  
  index.resize(numentries);
  for (int i = 0; i < numentries; i++) {
    index[i].id = ifs.readu32le();
    index[i].dataOffset = ifs.readu32le();
  }
  
  int datastart = chunkstart + dataOffset;
  for (int i = 0; i < numentries; i++) {
    ifs.seek(datastart + index[i].dataOffset);
    
    // this works, but we get the padding too
//    int sz = findChunkSize(i, filenameChunkOffset - dataOffset);

    // read size field from nitro header
    ifs.seekoff(8);
    int sz = ifs.readu32le();
    ifs.seekoff(-12);
    
    index[i].subfile.resize(sz);
    ifs.read((char*)(index[i].subfile.data()), sz);
  }
  
  ifs.seek(chunkstart + filenameChunkOffset);
  for (int i = 0; i < numentries; i++) {
    string str;
    char c;
    while ((c = ifs.get())) str += c;
    index[i].name = str;
    
//    cout << str << endl;
  }
//  cout << endl;
  
  // align to 0x200-byte boundary after filename chunk
//  ifs.seek(chunkstart + filenameChunkOffset + 0x100);
//  ifs.seekoff(0x100);
  ifs.alignToReadBoundary(fileTableByteAlignment);
}
  
int CaBinarchiveEntry::findChunkSize(int num, int extraStart) {
  if (num == index.size() - 1) return extraStart - index[num].dataOffset;
  
  return (index[num + 1].dataOffset - index[num].dataOffset);
}


}
