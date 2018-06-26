#include "ds/NitroPalette.h"
#include "util/TOfstream.h"
#include "util/TIfstream.h"
#include "util/TPngConversion.h"
#include <iostream>
#include <string>

using namespace BlackT;

namespace Nftred {


//int fuckshit::operator()(const BlackT::TColor& col) const {
//  return col.asInt();
//}
  
  
NitroPalette::NitroPalette() { }
  
void NitroPalette::read(BlackT::TStream& ifs) {
  int numcols = ifs.readu32le();
  for (int i = 0; i < numcols; i++) {
    int num = ifs.readu32le();
    int rawcol = ifs.readu32le();
    TColor col;
    col.fromInt(rawcol);
    indexToColor_[num] = col;
    colorToIndex_[col] = num;
  }
}

void NitroPalette::write(BlackT::TStream& ofs) const {
  ofs.writeu32le(numColors());
  for (std::map<int, BlackT::TColor>::const_iterator it
          = indexToColor_.cbegin();
       it != indexToColor_.cend();
       ++it) {
    ofs.writeu32le(it->first);
    ofs.writeu32le(it->second.asInt());
  }
}

void NitroPalette::saveToEditable(const char* prefix) const {
  std::string paletteName = std::string(prefix) + "-pal.bin";
  std::string paletteImageName = std::string(prefix) + "-pal.png";

  TOfstream palofs(paletteName.c_str(), std::ios_base::binary);
  write(palofs);
  
  TGraphic palpreview;
  generatePreview(palpreview);
  TPngConversion::graphicToRGBAPng(paletteImageName, palpreview);
}

void NitroPalette::loadFromEditable(const char* prefix) {
  std::string paletteName = std::string(prefix) + "-pal.bin";
  TIfstream palifs(paletteName.c_str(), std::ios_base::binary);
  read(palifs);
}
  
void NitroPalette::generatePreview(BlackT::TGraphic& dst) const {
  int numcols = numColors();
  int w = previewRectsPerRow * previewRectW;
  int h = (numcols / previewRectsPerRow) * previewRectH;
  
  dst.resize(w, h);
  dst.clearTransparent();
  
  int pos = 0;
  for (std::map<int, BlackT::TColor>::const_iterator it
          = indexToColor_.cbegin();
       it != indexToColor_.cend();
       ++it) {
    TColor col = it->second;
    int x = (pos % previewRectsPerRow) * previewRectW;
    int y = (pos / previewRectsPerRow) * previewRectH;
    
    dst.fillRect(x, y, previewRectW, previewRectH, col);
    
    ++pos;
  }
}
  
void NitroPalette::setColor(int index, BlackT::TColor color) {
  indexToColor_[index] = color;
  colorToIndex_[color] = index;
}

bool NitroPalette::hasColor(int index) const {
  return (indexToColor_.find(index) != indexToColor_.end());
}
  
int NitroPalette::numColors() const {
  return indexToColor_.size();
}

BlackT::TColor NitroPalette::color(int index) const {
  return indexToColor_.at(index);
}

int NitroPalette::indexOfColor(BlackT::TColor color) const {
//  for (std::map<BlackT::TColor, int>::const_iterator it
//    = colorToIndex_.cbegin();
//       it != colorToIndex_.cend();
//       ++it) {
//    std::cerr << (int)(it->first.r()) << " " << (int)(it->first.g()) << " "
//      << (int)(it->first.b()) << " " << (int)(it->first.a()) << std::endl;
//  }
//  std::cerr << (int)(color.r()) << " " << (int)(color.g()) << " "
//    << (int)(color.b()) << " " << (int)(color.a()) << std::endl;
  
  // hack, but should be good enough
  if (color.a() != TColor::fullAlphaOpacity) return 0;

  return colorToIndex_.at(color);
}


} 
