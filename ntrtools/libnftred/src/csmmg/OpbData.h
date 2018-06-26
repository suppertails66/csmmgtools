#ifndef OPBDATA_H
#define OPBDATA_H

#include "util/TArray.h"
#include "util/TByte.h"
#include "util/TStream.h"

namespace Nftred {


struct OpbEntry {
  int id;
  BlackT::TArray< BlackT::TArray<BlackT::TByte> > subdata;
};

class OpbData {
public:
  struct PrimaryIndexEntry {
    int id;
    int subchunkIndexOffset;
  };
  
  struct SubchunkIndexEntry {
    BlackT::TArray<int> dataOffsets;
  };
  
  OpbData();
  
  void readRaw(BlackT::TStream& ifs);
  void readAndChunkify(BlackT::TStream& ifs);
  
  void readFromFiles(const char* prefix);
  void writeToFiles(const char* prefix) const;
  void writeUsingChunks(BlackT::TStream& ofs) const;
  
  BlackT::TArray<OpbEntry> entries;
  
  BlackT::TArray<PrimaryIndexEntry> primaryIndex;
  
  BlackT::TArray<SubchunkIndexEntry> subchunkIndex;
  
  BlackT::TArray< BlackT::TArray< BlackT::TArray<char> > > dataChunks;
  
  const static int primaryIndexOffset;
  int subchunkIndexOffset;
  int dataChunkOffset;
  int extraOffset;
  
protected:
  
  int findNextChunkOffset(BlackT::TArray<SubchunkIndexEntry> subchunkIndex,
                          int chunkI,
                          int subchunkJ,
                          int dataChunkSize);
  
};


}


#endif
