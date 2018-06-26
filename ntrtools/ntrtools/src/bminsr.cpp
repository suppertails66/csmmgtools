#include "ds/NitroPalette.h"
#include "util/TGraphic.h"
#include "util/TArray.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TPngConversion.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Chou Soujuu Mecha MG BM graphic inserter" << endl;
    cout << "Usage: " << argv[0] << " <ingraphic> <inbm> <outfile>" << endl;
    
    return 0;
  }
  
  TGraphic g;
  TPngConversion::RGBAPngToGraphic(string(argv[1]), g);
  
  TIfstream ifs(argv[2], ios_base::binary);
  
//  int bpp = 8;
  
//  ifs.seek(0x12);
//  int height = ifs.readu32le();
//  int width = ifs.readu32le();
  
  ifs.seek(0x36);
  NitroPalette palette;
  int colorsInPalette = 256;
  for (int i = 0; i < colorsInPalette; i++) {
    int b = ifs.readu8le();
    int g = ifs.readu8le();
    int r = ifs.readu8le();
//    int a = 
    ifs.readu8le();
    
    palette.setColor(i, TColor(r, g, b,
      (i == 0) ? TColor::fullAlphaTransparency
        : TColor::fullAlphaOpacity));
  }
  
  ifs.seek(0);
  TBufStream ofs(0x100000);
  ofs.writeFrom(ifs, ifs.size());
  ifs.close();
  
  ofs.seek(0x436);
  for (int j = g.h() - 1; j >= 0; j--) {
    for (int i = 0; i < g.w(); i++) {
//      int value = (unsigned char)(ifs.readu8le());
//      g.setPixel(i, j, palette.color(value));
      TColor color = g.getPixel(i, j);
      ofs.writeu8le((char)(palette.indexOfColor(color)));
    }
  }
  
  ofs.alignToWriteBoundary(4);
  ofs.save(argv[3]);
  
  return 0;
}
