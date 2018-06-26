#include "csmmg/OpbData.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Extractor for file-based Chousoujuu Mecha MG OPB archives"
      << endl;
    cout << "Usage: " << argv[0] << " <infile> [outprefix]" << endl;
    return 0;
  }
  
  char* filename = argv[1];
  string prefix;
  if (argc >= 3) prefix = string(argv[2]);
  
  TIfstream ifs(filename, ios_base::binary);
  
  OpbData opb;
  opb.readAndChunkify(ifs);
//  opb.readFromFiles(prefix.c_str());
  
/*  int dataSize = ifs.size() - opb.dataChunkOffset;
  TArray<char> data;
  data.resize(dataSize);
  ifs.seek(opb.dataChunkOffset);
  ifs.read(data.data(), dataSize); */
  
//  TOfstream ofs((prefix + "index.bin").c_str(), ios_base::binary);
  opb.writeToFiles(prefix.c_str());
  
  return 0;
}
