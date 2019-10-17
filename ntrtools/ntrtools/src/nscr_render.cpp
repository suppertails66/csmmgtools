#include "ds/NcgrFile.h"
#include "ds/NcerFile.h"
#include "ds/NclrFile.h"
#include "ds/NscrFile.h"
#include "ds/NcgrTileSet.h"
#include "ds/NitroPalette.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TPngConversion.h"
#include "util/TStringConversion.h"
#include "util/TOpt.h"
#include <iostream>
#include <cstring>
#include <string>

using namespace std;
using namespace BlackT;
using namespace Nftred;
  
NitroPalette grayscale16;
NitroPalette grayscale256;

int main(int argc, char* argv[]) {
  if (argc < 5) {
    cout << "Tool to render Nintendo DS NCGR/NSCR/NCLR images"
         << endl;
    cout << "Usage: " << argv[0] << " <ncgr> <nclr> <nscr> <outprefix>"
      << endl;
    cout << "Options: " << endl;
    cout << "  -rs    Specifies rotation/scaling flag (default: 0)\n";
    
    return 0;
  }
  
  bool rotscale = false;
  char* rotscaleArg = TOpt::getOpt(argc, argv, "-rs");
  if (rotscaleArg != NULL) rotscale
    = TStringConversion::stringToInt(string(rotscaleArg)) != 0;

  //================================
  // Initialize grayscale palettes
  //================================
  
  for (int i = 0; i < 16; i++) {
    int value = (i << 4) | i;
    grayscale16.setColor(i, TColor(value, value, value,
      (i == 0) ? TColor::fullAlphaTransparency : TColor::fullAlphaOpacity));
//    grayscale16.setColor(i, TColor(value, value, value,
//      TColor::fullAlphaOpacity));
  }
  
  for (int i = 0; i < 256; i++) {
    int value = i;
    grayscale256.setColor(i, TColor(value, value, value,
      (i == 0) ? TColor::fullAlphaTransparency : TColor::fullAlphaOpacity));
//    grayscale256.setColor(i, TColor(value, value, value,
//      TColor::fullAlphaOpacity));
  }

  //================================
  // Load NCGR
  //================================
  
  char* ncgrName = argv[1];
  
  NcgrFile ncgr;
  {
    TIfstream ifs(ncgrName, ios_base::binary);
    ncgr.read(ifs);
  }
  
  NcgrTileSet tiles;
  tiles.fromNcgrFile(ncgr);
  
  //================================
  // Load palette
  //================================
  
  char* nclrName = argv[2];
  
  NclrFile nclr;
  {
    TIfstream ifs(nclrName, ios_base::binary);
    nclr.read(ifs);
  }
  
  //================================
  // Render NSCR
  //================================
  
  char* nscrName = argv[3];
  char* outPrefix = argv[4];

  NscrFile nscr;
  {
    TIfstream ifs(nscrName, ios_base::binary);
    nscr.read(ifs);
  }
  
  nscr.saveToEditableWithNclr(outPrefix, tiles.tiles_, nclr, rotscale);
  
  return 0;
}

