#include "ds/NitroTile.h"
#include "ds/NitroPalette.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TArray.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Chou Soujuu Mecha MG KAR-specific image extractor" << endl;
    cout << "Usage: " << argv[0] << " <inimg> <mode> [outprefix]" << endl;
    cout << endl;
    cout << "mode may be one of the following: " << endl;
    cout << "  t   Tile mode" << endl;
    cout << "  2   Tile mode 2" << endl;
    cout << "  b   Bitmap mode" << endl;
    
    return 0;
  }
  
  string imgname(argv[1]);
  
  char mode = *(argv[2]);
  
  string prefix;
  if (argc >= 4) prefix = string(argv[3]);
  
  string grpname = prefix + "-grp.png";
  string datname = prefix + "-dat.bin";
  string palname = prefix;
  
  TIfstream ifs(imgname.c_str(), ios_base::binary);
  
  int width = ifs.readu32le();
  int height = ifs.readu32le();
  int unknown1 = ifs.readu32le();
  int unknown2 = ifs.readu32le();
  
  int numcolors = 256;
  if (mode == '2') numcolors = 64;
  int bytesPerTile = NitroTile::pixelCount;
  int bpp = 8;
  
//  cout << dec << width << " " << height << endl;
  
  ifs.seek(0x10);
  NitroPalette pal;
  for (int i = 0; i < numcolors; i++) {
    int color = ifs.readu16le();
    int r = (color & 0x001F) << 3;
    int g = (color & 0x03E0) >> 2;
    int b = (color & 0x7C00) >> 7;
    int a = (i == 0) ? TColor::fullAlphaTransparency
      : TColor::fullAlphaOpacity;
    
    pal.setColor(i, TColor(r, g, b, a));
  }
  
  // mode 2 has 64 dummy entries that are used in-game to hold palettes
  // for character portraits
  if (mode == '2') {
    for (int i = 0; i < numcolors; i++) {
      int color = ifs.readu16le();
    }
  }
    
  TGraphic g(width, height);
  int tileW = width / NitroTile::width;
  int tileH = height / NitroTile::height;
  switch (mode) {
  case 't':
    for (int k = 0; k < tileH; k++) {
      for (int j = 0; j < tileW; j++) {
        TByte buffer[bytesPerTile];
        ifs.read((char*)buffer, bytesPerTile);
        NitroTile tile;
        tile.fromRaw(buffer, bpp);
        tile.toGraphicPalettized(g, pal,
                                 (j * NitroTile::width),
                                 (k * NitroTile::height));
//        g.setPixel(j, k, pal.color(value));
      }
    }
    break;
  case '2':
    // oam-style layout in 64x64 blocks: read 8 runs of 8 horizontal tiles,
    // assemble vertically, repeat for size of image
    // what is the fucking point of these 5 billion slightly different raw
    // image formats
    {
      int blocksW = (tileW / 8);
      int blocksH = (tileH / 8);
//      int numblocks = (blocksW * blocksH);
      for (int l = 0; l < blocksH; l++) {
        for (int k = 0; k < blocksW; k++) {
          for (int j = 0; j < 8; j++) {
            for (int i = 0; i < 8; i++) {
              int x = (k * 64) + (i * 8);
              int y = (l * 64) + (j * 8);
              
//              cout << x << " " << y << endl;
//              char c; cin >> c;
              
              TByte buffer[bytesPerTile];
              ifs.read((char*)buffer, bytesPerTile);
              NitroTile tile;
              tile.fromRaw(buffer, bpp);
              tile.toGraphicPalettized(g, pal,
                                       x, y);
            }
          }
        }
      }
      break;
    }
  case 'b':
    for (int k = 0; k < height; k++) {
      for (int j = 0; j < width; j++) {
        int value = (unsigned char)(ifs.get());
        g.setPixel(j, k, pal.color(value));
      }
    }
    break;
  default:
    cerr << "Unknown mode" << endl;
    return 1;
    break;
  }
  
  TPngConversion::graphicToRGBAPng(grpname, g);
  pal.saveToEditable(palname.c_str());
  TOfstream ofs(datname.c_str(), ios_base::binary);
  ofs.writeu32le(unknown1);
  ofs.writeu32le(unknown2);
  
  return 0;
}
