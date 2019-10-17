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
  if (argc < 3) {
    cout << "Tool to generate Nintendo DS NSCR/NCGR files from edited images"
         << endl;
    cout << "Usage: " << argv[0] << " <in_ncgr> <out_ncgr> [options]"
      << endl;
    cout << endl;
    cout << "Options: " << endl;
    cout << "  -nscr  Adds an editable NSCR prefix to the output.\n";
    cout << "         Followed by two arguments:\n";
    cout << "         1. the editable NSCR prefix\n";
    cout << "         2. the output NSCR binary\n";
//    cout << "  -p     Specifies NCLR palette file (grayscale if omitted)\n";
//    cout << "  -pn    Specifies index of sub-palette (default: 0)\n";
    
    return 0;
  }

  try {

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
    
    char* inNcgrName = argv[1];
    char* outNcgrName = argv[2];
    
    NcgrFile ncgr;
    {
      TIfstream ifs(inNcgrName, ios_base::binary);
      ncgr.read(ifs);
    }
    
    NcgrTileSet tiles;
    tiles.fromNcgrFile(ncgr);
    tiles.tiles_.resize(0);
    
    //================================
    // Load palette
    //================================
    
  /*  int palettenum = 0;
    TOpt::readNumericOpt(argc, argv, "-pn", &palettenum);
    
    NitroPalette pal = (tiles.tiles_.bpp() == 4)
      ? grayscale16 : grayscale256;
    
    char* paletteName = TOpt::getOpt(argc, argv, "-p");
    if (paletteName != NULL) {
      TIfstream ifs(paletteName, ios_base::binary);
      NclrFile nclr;
      nclr.read(ifs);
      if (palettenum < nclr.plttBlock.palettes.size()) {
        pal = nclr.plttBlock.palettes[palettenum];
      }
      else {
        cout << "Out-of-range palette index " << palettenum
          << " (max " << (nclr.plttBlock.palettes.size() - 1)
          << "); defaulting to grayscale" << endl;
      }
    }
    
    char* paletteName = argv[3]; */
    
    //================================
    // Process NSCRs
    //================================
    
    for (int i = 3; i < argc - 2; i++) {
      if (std::strcmp(argv[i], "-nscr") != 0) continue;
      
      char* prefix = argv[i + 1];
      char* screen = argv[i + 2];

  //    NscrFile nscr;
  //    {
  //      TIfstream ifs(screen, ios_base::binary);
  //      nscr.read(ifs);
  //    }
      
      NscrFile nscr;
  //    nscr.loadFromEditable(prefix);
      nscr.loadFromEditableWithNclr(prefix);
      
  //    nscr.generateTextBgFromEditable(prefix, tiles.tiles_, palettenum);
      nscr.generateTextBgFromEditableWithNclr(prefix, tiles.tiles_);
      
      {
        TBufStream ofs(0x100000);
        nscr.write(ofs);
        ofs.save(screen);
      }
    }
    
    //================================
    // Save NCGR
    //================================
    
    tiles.toNcgrFile(ncgr);
    {
      TBufStream ofs(0x100000);
      ncgr.write(ofs);
      ofs.save(outNcgrName);
    }  
    
    return 0;
  }
  catch (BlackT::TGenericException& e) {
    std::cerr << "Exception: " << e.problem() << std::endl;
    return 1;
  }
}

