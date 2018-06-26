#include "csmmg/OpbData.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include <cstring>
#include <iostream>
#include <string>

using namespace std;
using namespace BlackT;

namespace Nftred {


OpbData::OpbData() { }
  
void OpbData::readRaw(BlackT::TStream& ifs) {
  int filesize = ifs.size();

  int numEntries = ifs.readu32le();
  subchunkIndexOffset = ifs.readu32le();
  dataChunkOffset = ifs.readu32le();
  extraOffset = ifs.readu32le();
  
  // if dataOffset is outside the file limits, there's no data, and therefore
  // nothing for us to do
  if (dataChunkOffset >= filesize) return;
  
  // if there's no extra chunk, extraOffset will either be the same as
  // dataChunkOffset or the size of the file; reduce this to a common case
  if (extraOffset == dataChunkOffset) extraOffset = filesize;
  
  // Read the primary index (directly following the header)
//  TArray<PrimaryIndexEntry> primaryIndex;
  primaryIndex.resize(numEntries);
  for (int i = 0; i < numEntries; i++) {
    primaryIndex[i].id = ifs.readu32le();
    primaryIndex[i].subchunkIndexOffset = ifs.readu32le();
  }
  
  // Read the subchunk index
//  TArray<SubchunkIndexEntry> subchunkIndex;
  subchunkIndex.resize(numEntries);
  for (int i = 0; i < numEntries; i++) {
    int numSubchunks = ifs.readu32le();
    subchunkIndex[i].dataOffsets.resize(numSubchunks);
    for (int j = 0; j < numSubchunks; j++) {
      subchunkIndex[i].dataOffsets[j] = ifs.readu32le();
    }
  }
  
  // Read the data for each entry
/*  entries.resize(numEntries);
  for (int i = 0; i < numEntries; i++) {
    int numSubchunks = subchunkIndex[i].dataOffsets.size();
  
    entries[i].id = primaryIndex[i].id;
    entries[i].subdata.resize(numSubchunks);
    std::cout << "Entry " << i << " / " << entries[i].id << ": " << std::endl;
    for (int j = 0; j < numSubchunks; j++) {
      int dataOffset = subchunkIndex[i].dataOffsets[j];
      
      // subchunks may have an address of -1 (dummy entry)
      if (dataOffset < 0) continue;
      
      // these archives seem to have been designed to be read into memory
      // and accessed directly, not separated into their components;
      // there's no size field for data chunks, which means we have to figure
      // it out ourself
      int nextDataOffset
        = findNextChunkOffset(subchunkIndex, i, j, filesize - dataChunkOffset);
      
      int dataSize = nextDataOffset - dataOffset;
      ifs.seek(dataOffset);
      std::cout << "  " << dataOffset << ", " << dataSize << std::endl;
      
//      entries[i].subdata[j].resize(dataSize);
//      ifs.read((char*)(entries[i].subdata[j].data()), dataSize);
    }
  } */
}

void OpbData::readAndChunkify(BlackT::TStream& ifs) {
  readRaw(ifs);
  
  int filedataSize = ifs.size() - dataChunkOffset;
  TArray<char> data;
  data.resize(filedataSize);
  ifs.seek(dataChunkOffset);
  ifs.read(data.data(), filedataSize);
  
  dataChunks.resize(subchunkIndex.size());
  
  for (int i = 0; i < subchunkIndex.size(); i++) {
    int numEntries = subchunkIndex[i].dataOffsets.size();
      
    dataChunks[i].resize(numEntries);
    
    for (int j = 0; j < numEntries; j++) {
      int dataOffset = subchunkIndex[i].dataOffsets[j];
      int nextDataOffset
        = findNextChunkOffset(subchunkIndex, i, j, filedataSize);
      int subdataSize = nextDataOffset - dataOffset;
      dataChunks[i][j].resize(subdataSize);
      memcpy(dataChunks[i][j].data(), data.data() + dataOffset, subdataSize);
//      ifs.seek(dataOffset);
//      ifs.read(dataChunks[i][j].data(), subdataSize);
    }
    
  }
}
  
void OpbData::readFromFiles(const char* prefix) {
  TIfstream indexifs((string(prefix) + "index.bin").c_str(),
                     ios_base::binary);

  int numEntries = indexifs.readu32le();
  primaryIndex.resize(numEntries);
  subchunkIndex.resize(numEntries);
  dataChunks.resize(numEntries);
  
  for (int i = 0; i < numEntries; i++) {
    primaryIndex[i].id = indexifs.readu32le();
    primaryIndex[i].subchunkIndexOffset = 0xFFFFFFFF;
    
    int numSubchunks = indexifs.readu32le();
    subchunkIndex[i].dataOffsets.resize(numSubchunks);
    dataChunks[i].resize(numSubchunks);
    
    string filenameBase(prefix);
    filenameBase += TStringConversion::intToString(primaryIndex[i].id,
                                     TStringConversion::baseDec);
    filenameBase += "-";
    
    for (int j = 0; j < numSubchunks; j++) {
      subchunkIndex[i].dataOffsets[j] = 0xFFFFFFFF;
    
      string filename = filenameBase
        + TStringConversion::intToString(j,
                                       TStringConversion::baseDec);
                                       
      filename += ".bin";
      TIfstream ifs(filename.c_str(), ios_base::binary);
      dataChunks[i][j].resize(ifs.size());
      ifs.read(dataChunks[i][j].data(), dataChunks[i][j].size());
    }
  }
}

void OpbData::writeToFiles(const char* prefix) const {
  TOfstream indexofs((string(prefix) + "index.bin").c_str(),
                     ios_base::binary);

  int numEntries = subchunkIndex.size();
  indexofs.writeu32le(numEntries);
    
  for (int i = 0; i < numEntries; i++) {
    int numSubchunks = subchunkIndex[i].dataOffsets.size();
  
    indexofs.writeu32le(primaryIndex[i].id);
    indexofs.writeu32le(numSubchunks);
  
 //   for (int j = 0; j < numSubchunks; j++) {
 //     int dataOffset = subchunkIndex[i].dataOffsets[j];
 //     ofs.writeu32le(dataOffset);
 //   }
  }
  
  for (int i = 0; i < primaryIndex.size(); i++) {
    string filenameBase(prefix);
    filenameBase += TStringConversion::intToString(primaryIndex[i].id,
                                     TStringConversion::baseDec);
    filenameBase += "-";
    
    for (int j = 0; j < dataChunks[i].size(); j++) {
      string filename = filenameBase
        + TStringConversion::intToString(j,
                                       TStringConversion::baseDec);
      filename += ".bin";
      TOfstream ofs(filename.c_str(), ios_base::binary);
      ofs.write(dataChunks[i][j].data(), dataChunks[i][j].size());
    }
  }
  
}

void OpbData::writeUsingChunks(BlackT::TStream& ofs) const {
  int primaryIndexSize = primaryIndex.size() * 8;
  int subchunkIndexSize = subchunkIndex.size() * 4;
  for (int i = 0; i < subchunkIndex.size(); i++) {
    subchunkIndexSize += subchunkIndex[i].dataOffsets.size() * 4;
  }
  
  int fileStart = ofs.tell();
  int primaryIndexPos = fileStart + 0x10;
  int subIndexStart = primaryIndexPos + primaryIndexSize;
  int subIndexPos = subIndexStart;

  // 0x00: numentries
  ofs.writeu32le(primaryIndex.size());
  // 0x04: subchunk index offset
  ofs.writeu32le(subIndexPos);
  // 0x08: data offset -- fill in later
  ofs.seekoff(0x04);
  // 0x0C: ??? usually EOF -- fill in later
  ofs.seekoff(0x04);
  
  // primary index -- fill in later
  ofs.seekoff(primaryIndexSize);
  
  // subchunk index -- fill in later
  ofs.seekoff(subchunkIndexSize);
  
  int dataStart = ofs.tell();
  
  for (int i = 0; i < dataChunks.size(); i++) {
    int chunkStart = ofs.tell();
    int chunkStartRelative = chunkStart - dataStart;
    int subIndexStart = subIndexPos;
    
    ofs.seek(subIndexPos);
    ofs.writeu32le(dataChunks[i].size());
    subIndexPos += 4;
    ofs.seek(chunkStart);
    for (int j = 0; j < dataChunks[i].size(); j++) {
      int subchunkStart = ofs.tell();
      ofs.write(dataChunks[i][j].data(), dataChunks[i][j].size());
      int subchunkEnd = ofs.tell();
      
      ofs.seek(subIndexPos);
      ofs.writeu32le(subchunkStart - dataStart);
      subIndexPos += 4;
      ofs.seek(subchunkEnd);
    }
    
    int chunkEnd = ofs.tell();
    
    ofs.seek(primaryIndexPos);
    ofs.writeu32le(primaryIndex[i].id);
    primaryIndexPos += 4;
    ofs.writeu32le(subIndexStart - subIndexStart);
    
    ofs.seek(chunkEnd);
  }
  
  int dataEnd = ofs.tell();
  ofs.seek(fileStart + 0x08);
  ofs.writeu32le(dataStart);
  ofs.writeu32le(dataEnd);
  
  ofs.seek(dataEnd);
}
  
int OpbData::findNextChunkOffset(
                        BlackT::TArray<SubchunkIndexEntry> subchunkIndex,
                        int chunkI,
                        int subchunkJ,
                        int dataChunkSize) {
  ++subchunkJ;
  // search the subchunk index for the next valid data pointer
  for (int i = chunkI; i < (int)(subchunkIndex.size()); i++) {
    for ( ;
         subchunkJ < (int)(subchunkIndex[i].dataOffsets.size());
         subchunkJ++) {
      int offset = subchunkIndex[i].dataOffsets[subchunkJ];
      if (offset >= 0) return offset;
    }
    
    subchunkJ = 0;
  }
  
  // if we reach the end of the index, the remainder of the file is the data
  return dataChunkSize;
}


}
