#include "ds/PlttBlock.h"
#include "util/ByteConversion.h"
#include <iostream>
#include <cstring>

using namespace BlackT;

namespace Nftred {

  
PlttBlock::PlttBlock()
  : signature { 'P', 'L', 'T', 'T' },
    size(0),
    bpp(0),
    unknown1(0),
    unknown2(0),
    unknown3(0) { }
  
void PlttBlock::read(BlackT::TStream& ifs) {
  int startpos = ifs.tell();
  
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readu32le();
  
  int depth = ifs.readu16le();
  bpp = (depth == 3) ? 4 : 8;
  int colorsPerPalette = (bpp == 4) ? 16 : 256;
  
  unknown1 = ifs.readu16le();
  unknown2 = ifs.readu32le();
  unknown3 = ifs.readu32le();
  ifs.readu32le();
  
  int remaining = size - (ifs.tell() - startpos);
  
  int numPalettes = (remaining / (colorsPerPalette * 2));
  palettes.resize(numPalettes);
  for (int i = 0; i < numPalettes; i++) {
    for (int j = 0; j < colorsPerPalette; j++) {
      int color = ifs.readu16le();
      int r = (color & 0x001F) << 3;
      int g = (color & 0x03E0) >> 2;
      int b = (color & 0x7C00) >> 7;
      // doesn't play well with imported images (gimp throws away RGB
      // components of alpha pixels)
      int a = ((j == 0) ? TColor::fullAlphaTransparency
        : TColor::fullAlphaOpacity);
//      int a = TColor::fullAlphaOpacity;
      
      palettes[i].setColor(j, TColor(r, g, b, a));
    }
  }
  
}

void PlttBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeu32le(size);
  
  ofs.writeu16le((bpp == 4) ? 3 : 4);
  
  int colorsPerPalette = (bpp == 4) ? 16 : 256;
  
  ofs.writeu16le(unknown1);
  ofs.writeu32le(unknown2);
  ofs.writeu32le(unknown3);
  // FIXME: "memory size?"
  ofs.writeu32le(0x100);
  
  for (int i = 0; i < palettes.size(); i++) {
    for (int j = 0; j < colorsPerPalette; j++) {
      TColor color = palettes[i].color(j);
      
      int r = (int)(color.r() & 0xF8) >> 3;
      int g = (int)(color.g() & 0xF8) << 2;
      int b = (int)(color.b() & 0xF8) << 7;
      
      int raw = r | g | b;
      
      ofs.writeu16le(raw);
      
/*      // doesn't play well with imported images (gimp throws away RGB
      // components of alpha pixels)
      int a = ((j == 0) ? TColor::fullAlphaTransparency
        : TColor::fullAlphaOpacity);
//      int a = TColor::fullAlphaOpacity;
      
      palettes[i].setColor(j, TColor(r, g, b, a)); */
    }
  }
  
  ofs.alignToBoundary(4);
  int endpos = ofs.tell();
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


}
