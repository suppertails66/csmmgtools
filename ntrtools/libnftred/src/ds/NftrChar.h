#ifndef NFTRCHAR_H
#define NFTRCHAR_H


#include "util/TArray.h"
#include "util/TTwoDArray.h"
#include "util/TByte.h"
#include "util/TGraphic.h"
#include "util/TColor.h"

namespace Nftred {


class NftrChar {
public:
  NftrChar();
  
  void fromData(const BlackT::TByte* data, int datasize,
                int w, int h, int bpp,
                int bearingX__, int charWidth__, int advanceWidth__);
  
  void toData(BlackT::TByte* data, int bpp) const;
  
  int dataSize(int bpp) const;
  
  int bearingX() const;
  void setBearingX(int bearingX__);
  
  int charWidth() const;
  void setCharWidth(int charWidth__);
  
  int advanceWidth() const;
  void setAdvanceWidth(int advanceWidth__);
  
  void toGraphic(BlackT::TGraphic& dst, int bpp,
                 int x = 0, int y = 0) const;
  void fromGraphic(BlackT::TGraphic& src, int bpp);
  
  static BlackT::TColor valueToColor(int value, int bpp);
  static int colorToValue(const BlackT::TColor& color, int bpp);
  
  BlackT::TTwoDArray<BlackT::TByte> pixels;
protected:
  
  int bearingX_;
  int charWidth_;
  int advanceWidth_;
};


}


#endif
