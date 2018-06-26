#include "csmmg/OpbData.h"
#include "csmmg/CaBinarchive.h"
#include "csmmg/CaBinarchiveEntry.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TArray.h"
#include "util/TStringConversion.h"
#include "util/TFolderManip.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Extractor for Chou Soujuu Mecha MG ca.binarchive file" << endl;
    cout << "Usage: " << argv[0] << " <infile> [outprefix]" << endl;
    
    return 0; 
  }
  
  char* filename = argv[1];
  string prefix;
  if (argc >= 3) prefix = string(argv[2]);
  
  TIfstream ifs(filename, ios_base::binary);
  CaBinarchive arc;
  arc.readRaw(ifs);
  arc.saveToEditable(prefix.c_str());
  
//  CaBinarchive arc2;
//  arc2.loadFromEditable(prefix.c_str());
//  arc2.saveToEditable(prefix.c_str());
//  TBufStream testout(0x800000);
//  arc2.writeRaw(testout);
//  testout.save("test.binarchive");
/*  for (int i = 0; i < arc.entries.size(); i++) {

    for (int j = 0; j < arc.entries[i].index.size(); j++) {
      string filename
        = prefix + TStringConversion::intToString(j)
          + "/" + arc.entries[i].index[j].name;
//      cout << filename << endl;
      TFolderManip::createDirectoryForFile(filename);
      TOfstream ofs(filename.c_str(), ios_base::binary);
      ofs.write((char*)(arc.entries[i].index[j].subfile.data()),
                arc.entries[i].index[j].subfile.size());
    }
  } */
  
/*  int opbnum = 0;
  while (!ifs.nextIsEof()) {
    CaBinarchiveEntry arc;
    arc.readRaw(ifs);
    int opbStart = ifs.tell();
  
    OpbData opb;
    opb.readAndChunkify(ifs);
    
    // pull the file table from the extra chunk
    int fileTableOffset = opb.extraOffset;
    ifs.seek(opbStart + fileTableOffset);
    int numStrings = opb.dataChunks.size();
    vector<string> fileNames;
    for (int i = 0; i < numStrings; i++) {
      string str;
      char c;
      while ((c = ifs.get())) str += c;
      fileNames.push_back(str);
      
//      cout << str << endl;
    }
//    cout << endl;
    
    // the size of the extra chunk is 0x100 bytes plus however much padding
    // it takes to reach the next 0x100-byte boundary
    ifs.seek(opbStart + fileTableOffset + 0x100);
    ifs.alignToReadBoundary(fileTableByteAlignment);
    
//    cout << ifs.tell() << endl;

    for (int i = 0; i < arc.index.size(); i++) {
      string filename
        = prefix + TStringConversion::intToString(opbnum)
          + "/" + arc.index[i].name;
//      cout << filename << endl;
      TFolderManip::createDirectoryForFile(filename);
      TOfstream ofs(filename.c_str(), ios_base::binary);
      ofs.write((char*)(arc.index[i].subfile.data()),
                arc.index[i].subfile.size());
    }
//    cout << endl;
    
    ++opbnum;
  } */
  
  
  return 0;
}
