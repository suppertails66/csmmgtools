#include "util/TFolderManip.h"
#include "util/TIfstream.h"
#include "util/TArray.h"
#include "util/TStringEncoding.h"
#include "util/TCsv.h"
#include "csmmg/OpbData.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

using namespace std;
using namespace BlackT;
using namespace Nftred;

void escapeString(const string& src, string& dst) {
  int pos = 0;
  while (pos < src.size()) {
    if ((pos < src.size() - 1) && (src[pos] == '/')) {
      if (src[pos + 1] == 'n') {
        dst += '\n';
        pos += 2;
      }
      else {
        dst += src[pos++];
      }
    }
    else {
      dst += src[pos++];
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Extractor for Chousoujuu Mecha MG OPB archives" << endl;
    cout << "Usage: " << argv[0] << " <infile> [outprefix]" << endl;
    return 0;
  }
  
  char* filename = argv[1];
  string prefix;
  if (argc >= 3) prefix = string(argv[2]);
  
  TIfstream ifs(filename, ios_base::binary);
  
  OpbData opb;
  opb.readRaw(ifs);
  
  int dataSize = ifs.size() - opb.dataChunkOffset;
  TArray<char> data;
  data.resize(dataSize);
  ifs.seek(opb.dataChunkOffset);
  ifs.read(data.data(), dataSize);
  
  int pos = 0;
//  vector<string> strings;
//  vector<int> stringOffsets;
  map <int, string> offsetToString;
  while (pos < data.size()) {
//    stringOffsets.push_back(pos);
    string str;
    int amt = TStringEncoding::readSjisString(data.data() + pos, str);
    offsetToString[pos] = str;
    pos += amt;
//    strings.push_back(str);
  }
  
//  cout << strings.size() << endl;

  // map every string offset to everything that looks like a pointer to it
  map <int, map<int, bool> > stringOffsetsToPointerAddresses;
  
  for (int i = 0; i < opb.subchunkIndex.size(); i++) {
    for (int j = 0; j < opb.subchunkIndex[i].dataOffsets.size(); j++) {
    
      // ***
      // exception rules for specific files
      // ***
      
      // PartsDesc
//      if ((j == 3)) continue;
      
      // RoboDesc
      if (j == 2) continue;
      if (j > 5) continue;
      
      // ***
      // read
      // ***
    
      int offset = opb.subchunkIndex[i].dataOffsets[j];
      
      if (offset < 5) continue;
      
      // make sure this is actually a valid string pointer
      if (offsetToString.find(offset) == offsetToString.end()) continue;
      
      int pointerAddress
        = opb.primaryIndex[i].subchunkIndexOffset + opb.subchunkIndexOffset
          + 4 + (j * 4);
      stringOffsetsToPointerAddresses[offset][pointerAddress] = true;
    }
  }
  
  cout << "File,Offset,Pointers,Japanese,English" << endl;
  
  for (map<int, string>::iterator kt = offsetToString.begin();
       kt != offsetToString.end();
       ++kt) {
       
    cout << filename << ",";
    cout << hex << "0x" << kt->first + opb.dataChunkOffset << ",";
    
    map <int, map<int, bool> >::iterator it
      = stringOffsetsToPointerAddresses.find(kt->first);
    if (it != stringOffsetsToPointerAddresses.end()) {
      cout << hex << "0x" << it->second.size();
      for (map<int, bool>::iterator jt = it->second.begin();
           jt != it->second.end();
           ++jt) {
        cout << hex << " 0x" << jt->first;
      }
      cout << ",";
    }
    else {
      cout << hex << "0x0,";
    }
    
    string escaped;
    escapeString(kt->second, escaped);
    
    cout << "\"" << escaped << "\"" << ",";
    cout << "," << endl;
  
  }
  
/*  TCsv csv;
  csv.readSjis(ifs);
  
  for (int j = 0; j < csv.numRows(); j++) {
//    cout << j << endl;
    for (int i = 0; i < csv.numCols(); i++) {
      cout << "\"" << csv.cell(i, j) << "\"";
      if (i < (csv.numRows() - 1)) cout << ",";
    }
    cout << endl;
  } */
  
  return 0;
} 
