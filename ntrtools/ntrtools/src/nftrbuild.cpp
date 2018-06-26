#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TFolderManip.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "ds/NftrFile.h"
#include "ds/NftrFont.h"
#include <iostream>
#include <string>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "NFTR font builder" << endl;
    cout << "Usage: " << argv[0] << " <outfile> [inprefix] [padsize]" << endl;
    cout << "The font will be padded to [padsize] bytes, if provided." << endl;
    
    return 0;
  }
  
  string outfile(argv[1]);
  
  string prefix;
  if (argc >= 3) prefix = string(argv[2]);
  
  int padsize = -1;
  if (argc >= 4) padsize = TStringConversion::stringToInt(argv[3]);
  
  NftrFont font;
  font.load(prefix.c_str());
  
  NftrFile nftrFile;
  font.toNftrFile(nftrFile);
  TBufStream ofs(0x100000);
  nftrFile.write(ofs);
  if (padsize >= 0) {
    // 0x22224
    if (ofs.tell() < padsize) ofs.seek(padsize);
  }
  ofs.save(outfile.c_str());
  
  return 0;
}
