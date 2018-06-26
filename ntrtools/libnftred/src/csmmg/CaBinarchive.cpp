#include "csmmg/CaBinarchive.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include "util/TFolderManip.h"
#include <string>
#include <vector>

using namespace BlackT;

namespace Nftred {


CaBinarchive::CaBinarchive() { }
  
void CaBinarchive::readRaw(BlackT::TStream& ifs) {
  while (!ifs.nextIsEof()) {
    CaBinarchiveEntry arc;
    arc.readRaw(ifs);
    entries.push_back(arc);
  }
  
}

void CaBinarchive::writeRaw(BlackT::TStream& ofs) const {
  for (int i = 0; i < entries.size(); i++) {
    const CaBinarchiveEntry& entry = entries[i];
    
    int chunkStart = ofs.tell();
    
    int indexSize = entry.index.size() * 8;
    // skip over header
    ofs.seekoff(28 + indexSize);
    
    int dataStart = ofs.tell();
    
    std::vector<int> fileOffsets;
    fileOffsets.resize(entry.index.size());
    
    // write files
    for (int j = 0; j < entry.index.size(); j++) {
      const CaBinarchiveEntry::IndexEntry& subentry = entry.index[j];
      
      fileOffsets[j] = ofs.tell() - dataStart;
      ofs.write((char*)(subentry.subfile.data()),
                subentry.subfile.size());
      // maintain 32-bit alignment
      ofs.alignToBoundary(4);
    }
    
    int fileNameChunkOffset = ofs.tell();
    
    // write filenames
    for (int j = 0; j < entry.index.size(); j++) {
      ofs.writeCstr(entry.index[j].name.c_str());
    }
    
    ofs.alignToBoundary(0x200);
    int chunkEnd = ofs.tell();
    
    // fill in header
    ofs.seek(chunkStart);
    ofs.writeu32le(entry.unknown1);
    ofs.writeu32le(0x18);
    ofs.writeu32le(dataStart - chunkStart);
    ofs.writeu32le(fileNameChunkOffset - chunkStart);
    ofs.writeu32le(entry.unknown2);
    ofs.writeu32le(entry.unknown3);
    ofs.writeu32le(entry.index.size() * 2);
    for (int j = 0; j < entry.index.size(); j++) {
      ofs.writeu32le(entry.index[j].id);
      ofs.writeu32le(fileOffsets[j]);
    }
    
    ofs.seek(chunkEnd);
  }
}
  
void CaBinarchive::saveToEditable(const char* prefix) const {
  std::string indexName = std::string(prefix)
    + "index.bin";
  
  TOfstream idxofs(indexName.c_str(), std::ios_base::binary);
  
  
  
  idxofs.writeu32le(entries.size());
  
  for (int i = 0; i < entries.size(); i++) {
    const CaBinarchiveEntry& entry = entries[i];
    
    idxofs.writeu32le(i);
    idxofs.writeu32le(entry.unknown1);
    idxofs.writeu32le(entry.unknown2);
    idxofs.writeu32le(entry.unknown3);
    
    idxofs.writeu32le(entry.index.size());
    for (int j = 0; j < entry.index.size(); j++) {
      const CaBinarchiveEntry::IndexEntry& subentry = entry.index[j];
      
      idxofs.writeu32le(subentry.id);
      idxofs.writeCstr(subentry.name.c_str());
      
      // save file
      std::string filename
        = prefix + TStringConversion::intToString(i)
          + "/" + subentry.name;
//      cout << filename << endl;
      TFolderManip::createDirectoryForFile(filename);
      TOfstream ofs(filename.c_str(), std::ios_base::binary);
      ofs.write((char*)(subentry.subfile.data()),
                subentry.subfile.size());
    }
  }
}

void CaBinarchive::loadFromEditable(const char* prefix) {
  std::string indexName = std::string(prefix)
    + "index.bin";
  
  TIfstream idxifs(indexName.c_str(), std::ios_base::binary);
  
  int numEntries = idxifs.readu32le();
  entries.resize(numEntries);
  for (int i = 0; i < entries.size(); i++) {
    CaBinarchiveEntry& entry = entries[i];
    
//    int num = idxifs.readu32le();
    idxifs.readu32le();
    entry.unknown1 = idxifs.readu32le();
    entry.unknown2 = idxifs.readu32le();
    entry.unknown3 = idxifs.readu32le();
    
    int numSubentries = idxifs.readu32le();
    entry.index.resize(numSubentries);
    for (int j = 0; j < entry.index.size(); j++) {
      CaBinarchiveEntry::IndexEntry& subentry = entry.index[j];
      
      subentry.id = idxifs.readu32le();
      subentry.dataOffset = -1;
      idxifs.readCstrString(subentry.name);
      
      TIfstream fileifs((std::string(prefix)
        + TStringConversion::intToString(i)
        + "/"
        + subentry.name).c_str(), std::ios_base::binary);
      subentry.subfile.resize(fileifs.size());
      fileifs.read((char*)(subentry.subfile.data()),
                   subentry.subfile.size());
    }
  }
  
  
}


}
