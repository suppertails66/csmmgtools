#include "ds/NitroTile.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <iostream>

using namespace BlackT;

namespace Nftred {


NitroTile::NitroTile() {
  data_.resize(width, height);
}

bool NitroTile::operator==(const NitroTile& tile) const {
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      if (getPixel(i, j) != tile.getPixel(i, j)) return false;
    }
  }
  
  return true;
}

bool NitroTile::operator!=(const NitroTile& tile) const {
  return !(*this == tile);
}

BlackT::TByte NitroTile::getPixel(int x, int y) const {
  if ((x >= width) || (y >= height)) {
    throw TGenericException(T_SRCANDLINE,
                            "NitroTile::getPixel()",
                            "Out-of-range access: "
                              + TStringConversion::intToString(x)
                              + ", "
                              + TStringConversion::intToString(y));
  }
  
  return data_.data(x, y);
}

void NitroTile::setPixel(int x, int y, BlackT::TByte value) {
  if ((x >= width) || (y >= height)) {
    throw TGenericException(T_SRCANDLINE,
                            "NitroTile::setPixel()",
                            "Out-of-range access: "
                              + TStringConversion::intToString(x)
                              + ", "
                              + TStringConversion::intToString(y));
  }
  
  data_.data(x, y) = value;
}
  
int NitroTile::fromRaw(const BlackT::TByte* src, int bpp) {
  for (int i = 0; i < pixelCount; ) {
    switch (bpp) {
    case 4:
    {
      TByte pixel1 = (*src & 0x0F);
      TByte pixel2 = (*src & 0xF0) >> 4;
      setPixel(i % width, i / width, pixel1);
      setPixel((i + 1) % width, (i + 1) / width, pixel2);
      ++src;
      i += 2;
    }
      break;
    case 8:
    {
      TByte pixel1 = (*src & 0xFF);
      setPixel(i % width, i / width, pixel1);
      ++src;
      ++i;
    }
      break;
    default:
      throw TGenericException(T_SRCANDLINE,
                              "NitroTile::fromRaw()",
                              "Illegal BPP: "
                                + TStringConversion::intToString(bpp));
      break;
    }
  }
  
  return (pixelCount / (8 / bpp));
}

int NitroTile::toRaw(BlackT::TByte* dst, int bpp) const {
  for (int i = 0; i < pixelCount; ) {
    switch (bpp) {
    case 4:
    {
      TByte pixel1 = getPixel(i % width, i / width);
      TByte pixel2 = getPixel((i + 1) % width, (i + 1) / width);
      *dst = (pixel1 & 0x0F);
      *dst |= (pixel2 & 0x0F) << 4;
      ++dst;
      i += 2;
    }
      break;
    case 8:
    {
      TByte pixel1 = getPixel(i % width, i / width);
      *dst = (pixel1 & 0xFF);
      ++dst;
      ++i;
    }
      break;
    default:
      throw TGenericException(T_SRCANDLINE,
                              "NitroTile::toRaw()",
                              "Illegal BPP: "
                                + TStringConversion::intToString(bpp));
      break;
    }
  }
  
  return (pixelCount / (8 / bpp));
}
  
void NitroTile::toGraphicPalettized(BlackT::TGraphic& dst,
                         const NitroPalette& palette,
                         int xoffset,
                         int yoffset) const {
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      int x = i + xoffset;
      int y = j + yoffset;
      dst.setPixel(x, y, palette.color(data_.data(i, j)));
    }
  }
}
  
bool NitroTile::fromGraphicPalettized(const BlackT::TGraphic& src,
                         const NitroPalette& palette,
                         int xoffset,
                         int yoffset) {
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      int x = i + xoffset;
      int y = j + yoffset;
      
      TColor color = src.getPixel(x, y);
      if (!palette.hasIndex(color)) return false;
      
      data_.data(i, j) = palette.indexOfColor(color);
    }
  }
  
  return true;
}

void NitroTile::flipHorizontal() {
  for (int j = 0; j < height; j++) {
    int start = 0;
    int end = width - 1;
    
    while (start < end) {
      BlackT::TByte first = getPixel(start, j);
      BlackT::TByte second = getPixel(end, j);
      
      setPixel(start, j, second);
      setPixel(end, j, first);
      
      ++start;
      --end;
    }
  }
}

void NitroTile::flipVertical() {
  for (int i = 0; i < width; i++) {
    int start = 0;
    int end = height - 1;
    
    while (start < end) {
      BlackT::TByte first = getPixel(i, start);
      BlackT::TByte second = getPixel(i, end);
      
      setPixel(i, start, second);
      setPixel(i, end, first);
      
      ++start;
      --end;
    }
  }
}


} 
