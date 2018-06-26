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
    cout << "Chou Soujuu Mecha MG KAR-specific image creator" << endl;
    cout << "Usage: " << argv[0] << " <outimg> <mode> [inprefix]" << endl;
    cout << endl;
    cout << "mode may be one of the following: " << endl;
//    cout << "  t   Tile mode" << endl;
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
  
  TIfstream datifs(datname.c_str(), ios_base::binary);
  
//  cerr << datifs.size() << endl;
  
//  int oldwidth = 
  datifs.readu32le();
//  int oldheight = 
  datifs.readu32le();
  int oldunknown1 = datifs.readu32le();
  int oldunknown2 = datifs.readu32le();
  
  int numcolors = 256;
  if (mode == '2') numcolors = 64;
  int bytesPerTile = NitroTile::pixelCount;
  int bpp = 8;
  
  TGraphic g;
  TPngConversion::RGBAPngToGraphic(grpname, g);
  
  NitroPalette pal;
  pal.loadFromEditable(palname.c_str());
  
  TOfstream ofs(imgname.c_str());
  
  ofs.writeu32le(g.w());
  ofs.writeu32le(g.h());
  ofs.writeu32le(oldunknown1);
  ofs.writeu32le(oldunknown2);
  
  for (int i = 0; i < numcolors; i++) {
    TColor color = pal.color(i);
    int rawCol = 0;
    
    int r = (unsigned char)(color.r());
    int g = (unsigned char)(color.g());
    int b = (unsigned char)(color.b());
    
    rawCol |= (r >> 3);
    rawCol |= (g << 2);
    rawCol |= (b << 7);
    
    ofs.writeu16le(rawCol);
  }
  
  if (mode == '2') {
    for (int i = 0; i < numcolors; i++) {
      ofs.writeu16le(0x0000);
    }
  }
    
//  TGraphic g(width, height);
//  int tileW = width / NitroTile::width;
//  int tileH = height / NitroTile::height;
  switch (mode) {
/*  case 't':
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
    break; */
  case 'b':
    for (int k = 0; k < g.h(); k++) {
      for (int j = 0; j < g.w(); j++) {
        TColor color = g.getPixel(j, k);
        int value = pal.indexOfColor(color);
        ofs.put((unsigned char)value);
      }
    }
    break;
  case '2':
    // oam-style layout in 64x64 blocks: read 8 runs of 8 horizontal tiles,
    // assemble vertically, repeat for size of image
    // what is the fucking point of these 5 billion slightly different raw
    // image formats
    {
      int blocksW = (g.w() / 64);
      int blocksH = (g.h() / 64);
      for (int l = 0; l < blocksH; l++) {
        for (int k = 0; k < blocksW; k++) {
          for (int j = 0; j < 8; j++) {
            for (int i = 0; i < 8; i++) {
              int x = (k * 64) + (i * 8);
              int y = (l * 64) + (j * 8);
              
              NitroTile tile;
              tile.fromGraphicPalettized(g, pal,
                                       x, y);
              
              
              TByte buffer[bytesPerTile];
              tile.toRaw(buffer, bpp);
              
              ofs.write((char*)buffer, bytesPerTile);
            }
          }
        }
      }
      break;
    }
  default:
    cerr << "Unknown mode" << endl;
    return 1;
    break;
  }
  
  return 0;
}
