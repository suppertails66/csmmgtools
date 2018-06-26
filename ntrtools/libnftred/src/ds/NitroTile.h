#ifndef NITROTILE_H
#define NITROTILE_H


#include "ds/NitroPalette.h"
#include "util/TTwoDArray.h"
#include "util/TByte.h"
#include "util/TGraphic.h"

namespace Nftred {


class NitroTile {
public:
  const static int width = 8;
  const static int height = 8;
  const static int pixelCount = width * height;
  
  NitroTile();
  
  BlackT::TByte getPixel(int x, int y) const;
  void setPixel(int x, int y, BlackT::TByte value);
  
  int fromRaw(const BlackT::TByte* src, int bpp);
  int toRaw(BlackT::TByte* dst, int bpp) const;
  
  void toGraphicPalettized(BlackT::TGraphic& dst,
                           const NitroPalette& palette,
                           int xoffset = 0,
                           int yoffset = 0) const;
  
  void fromGraphicPalettized(const BlackT::TGraphic& src,
                           const NitroPalette& palette,
                           int xoffset = 0,
                           int yoffset = 0);
//  void fromGraphicPalettized(const BlackT::TGraphic& src,
//                             const NitroPalette& palette) const;
  
protected:
  BlackT::TTwoDArray<BlackT::TByte> data_;
};


}


#endif
