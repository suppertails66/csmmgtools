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
  if (argc < 2) {
    cout << "Chou Soujuu Mecha MG KAR archive extractor" << endl;
    cout << "Usage: " << argv[0] << " <kar> [outprefix]" << endl;
//    cout << endl;
//    cout << "Mode may be one of the following: " << endl;
//    cout << "  t   Tile mode" << endl;
//    cout << "  b   Bitmap mode" << endl;
    
    return 0;
  }
  
  string karname(argv[1]);
  
//  char mode = *(argv[2]);
  
  string prefix;
  if (argc >= 3) prefix = string(argv[2]);
  
  TOfstream idxofs((prefix + "index.bin").c_str(), ios_base::binary);
  
//  idxofs.writeu32le(mode);
  
  TIfstream karifs(karname.c_str(), ios_base::binary);
  karifs.seek(0x14);
  int numentries = karifs.readu32le();
  
  karifs.seek(0x20);
  vector<int> offsets;
  offsets.resize(numentries);
  for (int i = 0; i < numentries; i++) {
    offsets[i] = karifs.readu32le();
  }
  
  idxofs.writeu32le(numentries);
  
  for (int i = 0; i < offsets.size(); i++) {
    int offset = offsets[i];
    
    karifs.seek(offset);
    
    int size1 = karifs.readu32le();
    int unknown1 = karifs.readu32le();
    int size2 = karifs.readu32le();
    int unknown2 = karifs.readu32le();
    
//    cout << hex << size1 << " " << unknown1 << " "
//         << size2 << " " << unknown2 << endl;
         
    if (size1 != size2) {
      cerr << "WARNING: Size mismatch at " << hex << offset << endl;
    }
    
    TArray<char> content;
    content.resize(size1);
    karifs.read(content.data(), size1);
    
    string outname = prefix + TStringConversion::intToString(i)
      + ".bin";
    TOfstream ofs(outname.c_str(), ios_base::binary);
    ofs.write(content.data(), size1);
    
    idxofs.writeu32le(offset);
    
    idxofs.writeu32le(size1);
    idxofs.writeu32le(unknown1);
    idxofs.writeu32le(size2);
    idxofs.writeu32le(unknown2);
  }
  
/*  for (int i = 0; i < offsets.size(); i++) {
    string grpname = prefix + TStringConversion::intToString(i) + "-grp.png";
    string palname = prefix + TStringConversion::intToString(i);
    
    int offset = offsets[i];
    
    karifs.seek(offset + 0x10);
    int width = karifs.readu32le();
    int height = karifs.readu32le();
    
    int numcolors = 256;
    int bytesPerTile = NitroTile::pixelCount;
    int bpp = 8;
    
    cout << hex << offset << " " << dec << width << " " << height << endl;
    
    karifs.seek(offset + 0x20);
    NitroPalette pal;
    for (int i = 0; i < numcolors; i++) {
      int color = karifs.readu16le();
      int r = (color & 0x001F) << 3;
      int g = (color & 0x03E0) >> 2;
      int b = (color & 0x7C00) >> 7;
      int a = (i == 0) ? TColor::fullAlphaTransparency
        : TColor::fullAlphaOpacity;
      
      pal.setColor(i, TColor(r, g, b, a));
    }
      
    TGraphic g(width, height);
    int tileW = width / NitroTile::width;
    int tileH = height / NitroTile::height;
    switch (mode) {
    case 't':
      for (int k = 0; k < tileH; k++) {
        for (int j = 0; j < tileW; j++) {
          TByte buffer[bytesPerTile];
          karifs.read((char*)buffer, bytesPerTile);
          NitroTile tile;
          tile.fromRaw(buffer, bpp);
          tile.toGraphicPalettized(g, pal,
                                   (j * NitroTile::width),
                                   (k * NitroTile::height));
  //        g.setPixel(j, k, pal.color(value));
        }
      }
      break;
    case 'b':
      for (int k = 0; k < height; k++) {
        for (int j = 0; j < width; j++) {
          int value = (unsigned char)(karifs.get());
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
    
    idxofs.writeu32le(offset);
  } */
  
  return 0;
}
