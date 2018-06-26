#include "ds/NitroPalette.h"
#include "util/TGraphic.h"
#include "util/TArray.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TPngConversion.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Chou Soujuu Mecha MG BM graphic extractor" << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile>" << endl;
    
    return 0;
  }
  
  TIfstream ifs(argv[1], ios_base::binary);
  
//  int bpp = 8;
  
  ifs.seek(0x12);
  int height = ifs.readu32le();
  int width = ifs.readu32le();
  
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
  
  TGraphic g(width, height);
  for (int j = height - 1; j >= 0; j--) {
    for (int i = 0; i < width; i++) {
      int value = (unsigned char)(ifs.readu8le());
      g.setPixel(i, j, palette.color(value));
    }
  }
  
  TPngConversion::graphicToRGBAPng(string(argv[2]), g);
  
  TGraphic pal;
  palette.generatePreview(pal);
  
  TPngConversion::graphicToRGBAPng(string(argv[2]) + "-pal.png", pal);
  
  
  return 0;
}
