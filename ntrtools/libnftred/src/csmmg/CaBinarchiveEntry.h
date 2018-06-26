#ifndef CABINARCHIVEENTRY_H
#define CABINARCHIVEENTRY_H


#include "util/TArray.h"
#include "util/TByte.h"
#include "util/TStream.h"
#include <vector>

namespace Nftred {


class CaBinarchiveEntry {
public:
  struct IndexEntry {
    int id;
    int dataOffset;
    std::string name;
    BlackT::TArray<BlackT::TByte> subfile;
  };
  
  const static int fileTableByteAlignment = 0x200;
  
  CaBinarchiveEntry();
  
  void readRaw(BlackT::TStream& ifs);
  
  void readFromFiles(const char* prefix);
  void writeToFiles(const char* prefix) const;
  
  int unknown1;
  int unknown2;
  int unknown3;
  std::vector<IndexEntry> index;
  
protected:
  int findChunkSize(int num, int extraStart);
  
};


}


#endif
