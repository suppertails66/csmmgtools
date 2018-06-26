#include "ds/NcerFile.h"
#include "ds/NftrFile.h"
#include "ds/NftrFont.h"
#include "util/TIfstream.h"
#include "util/TBufStream.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
/*  TIfstream ifs("0-2.bin", ios_base::binary);
  
  NcerFile ncerFile;
  ncerFile.read(ifs);
  
  CebkBlock& cebk = ncerFile.cebkBlock;
  for (int j = 0; j < 3; j++) {
    CebkBankEntry& entry = cebk.banks[9 + j];
    for (int i = 0; i < entry.oams.size(); i++) {
      entry.oams[i].xpos -= 4;
    }
  }
  
  TBufStream ofs(0x100000);
  ncerFile.write(ofs);
  ofs.save("0-2_update.bin"); */
  
  if (argc < 3) {
    cout << "Usage: " << argv[0] << " <inprefix> <outprefix>" << endl;
    
    return 0;
  }
  
  NftrFont font;
  font.load(argv[1]);
  
  for (int i = 6953; i < 7014; i++) {
    NftrChar& c = font.chars_[i];
    int lastCol = 0;
    for (int i = 0; i < c.pixels.w(); i++) {
      for (int j = 0; j < c.pixels.h(); j++) {
        if (c.pixels.data(i, j) != 0) {
          lastCol = i;
          break;
        }
      }
    }
    
//    cout << i << " " << lastRow << endl;

    c.setCharWidth(lastCol + 1);
    c.setAdvanceWidth(lastCol + 2);
    c.setBearingX(0);
  }
  
  font.save(argv[2]);
  
  return 0;
}
