#include "ds/NitroTile.h"
#include "ds/NitroPalette.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
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

const static int sectionByteAlignment = 0x10;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Chou Soujuu Mecha MG KAR archive inserter" << endl;
    cout << "Usage: " << argv[0] << " <inkar> <outcar> [inprefix]" << endl;
//    cout << endl;
//    cout << "Mode may be one of the following: " << endl;
//    cout << "  t   Tile mode" << endl;
//    cout << "  b   Bitmap mode" << endl;
    
    return 0;
  }
  
  string inkarname(argv[1]);
  string outkarname(argv[2]);
  
  string prefix;
  if (argc >= 4) prefix = string(argv[3]);
  
  TIfstream idxifs((prefix + "index.bin").c_str(), ios_base::binary);
  
//  idxofs.writeu32le(mode);

  int numEntries = idxifs.readu32le();
  int startOffset = idxifs.readu32le();
  idxifs.seekoff(-4);
  
  TIfstream karifs(inkarname.c_str(), ios_base::binary);
  TBufStream karofs(0x800000);
  
  // copy original header
  karofs.writeFrom(karifs, startOffset);
  
  for (int i = 0; i < numEntries; i++) {
    int offset = karofs.tell();
    
//    int oldOffset = 
    idxifs.readu32le();
//    int oldsize1 = 
    idxifs.readu32le();
    int oldunknown1 = idxifs.readu32le();
//    int oldsize2 = 
    idxifs.readu32le();
    int oldunknown2 = idxifs.readu32le();
    
    TIfstream ifs(
      (prefix + TStringConversion::intToString(i) + ".bin").c_str(),
      ios_base::binary);
    int newsize = ifs.size();
    
    karofs.writeu32le(newsize);
    karofs.writeu32le(oldunknown1);
    karofs.writeu32le(newsize);
    karofs.writeu32le(oldunknown2);
    
    karofs.writeFrom(ifs, newsize);
    
    karofs.alignToWriteBoundary(sectionByteAlignment);
    
    int endOffset = karofs.tell();
    
    // update index
    karofs.seek(0x20 + (i * 4));
    karofs.writeu32le(offset);
    
    karofs.seek(endOffset);
  }
  
  karofs.save(outkarname.c_str());
  
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
