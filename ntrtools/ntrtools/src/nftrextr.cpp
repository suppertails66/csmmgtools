#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TFolderManip.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "ds/NftrFile.h"
#include "ds/NftrFont.h"
#include <string>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "NFTR font extractor" << endl;
    cout << "Usage: " << argv[0] << " <font> [outprefix]" << endl;
    
    return 0;
  }
  
  string prefix;
  if (argc >= 3) prefix = string(argv[2]);

  TIfstream ifs(argv[1], ios_base::binary);
  NftrFile nftrFile;
  nftrFile.read(ifs);
  
  NftrFont font;
  font.fromNftrFile(nftrFile);
  
  font.save(prefix.c_str());
  
  return 0;
}
