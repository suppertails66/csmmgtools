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
    cout << "Archiver for Chou Soujuu Mecha MG ca.binarchive file" << endl;
    cout << "Usage: " << argv[0] << " <outfile> [inprefix]" << endl;
    
    return 0; 
  }
  
  char* filename = argv[1];
  string prefix;
  if (argc >= 3) prefix = string(argv[2]);
  
  TIfstream ifs(filename, ios_base::binary);
  CaBinarchive arc;
  
  arc.loadFromEditable(prefix.c_str());
  TBufStream ofs(0x800000);
  arc.writeRaw(ofs);
  ofs.save(filename);
  
  return 0;
}
