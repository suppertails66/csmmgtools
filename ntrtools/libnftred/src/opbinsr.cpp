#include "util/TFolderManip.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TArray.h"
#include "util/TStringEncoding.h"
#include "util/TStringConversion.h"
#include "util/ByteConversion.h"
#include "util/TCsv.h"
#include "csmmg/OpbData.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cstring>

using namespace std;
using namespace BlackT;
using namespace Nftred;

// adding an additional zero to this segfaults on program start
// why??
const int stringTableBufferSize = 0x100000;

void unescapeString(const string& src, string& dst) {
  int pos = 0;
  while (pos < src.size()) {
    if ((src[pos] == '\n')) {
      dst += "/n";
      ++pos;
    }
    else {
      dst += src[pos++];
    }
  }
}

int nextIntFromIss(istringstream& iss) {
  string temp;
  iss >> temp;
  return TStringConversion::stringToInt(temp);
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Inserter for Chousoujuu Mecha MG OPB archives" << endl;
    cout << "Usage: " << argv[0] << " <opb> <csv> <outfile>" << endl;
    return 0;
  }
  
  char* opbFilename = argv[1];
  char* csvFilename = argv[2];
  char* outFilename = argv[3];
  
  TIfstream ifs(csvFilename, ios_base::binary);
  TCsv csv;
  csv.readSjis(ifs);
  ifs.close();
  
  ifs.open(opbFilename, ios_base::binary);
  TArray<char> opbFile;
  opbFile.resize(ifs.size());
  ifs.read(opbFile.data(), opbFile.size());
  ifs.close();
  
  // start of string table = first entry's offset
  int stringTableStart = TStringConversion::stringToInt(csv.cell(1, 1));
  
  char buffer[stringTableBufferSize];
  int putpos = 0;
  for (int j = 1; j < csv.numRows(); j++) {
    // Write English string to buffer
    const string& engstr = csv.cell(4, j);
    string unescaped;
    unescapeString(engstr, unescaped);
    strcpy(buffer + putpos, unescaped.c_str());
    
    // Update pointers
    istringstream iss(csv.cell(2, j));
    int numPointers = nextIntFromIss(iss);
    for (int i = 0; i < numPointers; i++) {
      int pointer = nextIntFromIss(iss);
      char* dst = opbFile.data() + pointer;
      ByteConversion::toBytes(putpos, dst, 4,
        EndiannessTypes::little, SignednessTypes::nosign);
    }
    
    putpos += unescaped.size() + 1;
  }
  
  TOfstream ofs(outFilename, ios_base::binary);
  // write everything up to the string table
  ofs.write(opbFile.data(), stringTableStart);
  // write the new string table
  ofs.write(buffer, putpos);
  ofs.close();
  
  return 0;
} 
