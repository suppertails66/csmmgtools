#include "ds/NftrChar.h"
#include "util/TStringConversion.h"
#include "util/MiscMath.h"
#include "exception/TGenericException.h"
#include <iostream>
#include <cmath>

using namespace BlackT;

namespace Nftred {


NftrChar::NftrChar()
  : bearingX_(0),
    charWidth_(0),
    advanceWidth_(0) { }
  
void NftrChar::fromData(const TByte* data, int datasize,
              int w, int h, int bpp,
              int bearingX__, int charWidth__, int advanceWidth__) {
//  if ((bpp != 1) && (bpp != 2) && (bpp != 4) && (bpp != 8)) {
//  }
  
  int mask;
  int shift;
  switch (bpp) {
  case 1:
    mask = 0x80;
    shift = 1;
    break;
  case 2:
    mask = 0xC0;
    shift = 2;
    break;
  case 4:
    mask = 0xF0;
    shift = 4;
    break;
  case 8:
    mask = 0xFF;
    shift = 8;
    break;
  default:
    throw TGenericException(T_SRCANDLINE,
                            "NftrChar::fromData()",
                            std::string("Invalid BPP: ")
                              + TStringConversion::toString(bpp));
    break;
  }
  
  int pixelpos = 0;
  int numpixels = w * h;
  int availablepixels = datasize * (8 / bpp);
  if (availablepixels < numpixels) numpixels = availablepixels;
  pixels.resize(w, h);
  pixels.fill(0);
  
//  std::cout << "numpixels: " << numpixels << std::endl;
  
  while (pixelpos < numpixels) {
    // read each byte
    int bits = 7;
    for (int m = mask; m > 0; m >>= shift) {
      int next = (*data & m) >> bits;
      bits -= shift;
      int x = (pixelpos % w);
      int y = (pixelpos / w);
      pixels.data(x, y) = next;
      ++pixelpos;
      
      if (pixelpos >= numpixels) break;
    }
    ++data;
  }
  
  bearingX_ = bearingX__;
  charWidth_ = charWidth__;
  advanceWidth_ = advanceWidth__;
}
  
void NftrChar::toData(BlackT::TByte* data, int bpp) const {
  
  int mask;
  int shift;
  switch (bpp) {
  case 1:
    mask = 0x80;
    shift = 1;
    break;
  case 2:
    mask = 0xC0;
    shift = 2;
    break;
  case 4:
    mask = 0xF0;
    shift = 4;
    break;
  case 8:
    mask = 0xFF;
    shift = 8;
    break;
  default:
    throw TGenericException(T_SRCANDLINE,
                            "NftrChar::toData()",
                            std::string("Invalid BPP: ")
                              + TStringConversion::toString(bpp));
    break;
  }
  
  int pixelpos = 0;
  int numpixels = pixels.w() * pixels.h();
  
//  std::cout << "numpixels: " << numpixels << std::endl;
  
  while (pixelpos < numpixels) {
    // write each byte
    *data = 0;
    int bits = 7;
    for (int m = mask; m > 0; m >>= shift) {
      int x = (pixelpos % pixels.w());
      int y = (pixelpos / pixels.w());
      
      int value = pixels.data(x, y);
      value <<= bits;
      *data |= value;
      
      bits -= shift;
      ++pixelpos;
      
      if (pixelpos >= numpixels) break;
    }
    ++data;
  }
}
  
int NftrChar::dataSize(int bpp) const {
  int canvasSize = (pixels.w() * pixels.h());
  int canvasBytes = canvasSize / (9 - bpp);
  if ((canvasSize % (9 - bpp)) != 0) ++canvasBytes;
  return canvasBytes;
}

int NftrChar::bearingX() const {
  return bearingX_;
}

void NftrChar::setBearingX(int bearingX__) {
  bearingX_ = bearingX__;
}

int NftrChar::charWidth() const {
  return charWidth_;
}

void NftrChar::setCharWidth(int charWidth__) {
  charWidth_ = charWidth__;
}

int NftrChar::advanceWidth() const {
  return advanceWidth_;
}

void NftrChar::setAdvanceWidth(int advanceWidth__) {
  advanceWidth_ = advanceWidth__;
}
  
void NftrChar::toGraphic(BlackT::TGraphic& dst, int bpp,
                         int x, int y) const {
//  dst.resize(pixels.w(), pixels.h());
  if ((dst.w() < pixels.w()) || (dst.h() < pixels.h())) {
    dst.resize(pixels.w(), pixels.h());
  }
  
  for (int j = 0; j < pixels.h(); j++) {
    for (int i = 0; i < pixels.w(); i++) {
      dst.setPixel(i + x, j + y, valueToColor(pixels.data(i, j), bpp));
    }
  }
}
  
void NftrChar::fromGraphic(BlackT::TGraphic& src, int bpp) {
  pixels.resize(src.w(), src.h());
  
  for (int j = 0; j < src.h(); j++) {
    for (int i = 0; i < src.w(); i++) {
      pixels.data(i, j) = colorToValue(src.getPixel(i, j), bpp);
    }
  }
}
  
BlackT::TColor NftrChar::valueToColor(int value, int bpp) {
  int level = (0xFF / ((0x01 << bpp) - 1)) * value;
  // invert values so zero is white and higher colors are darker
  level = (0xFF - level);
  return BlackT::TColor(level, level, level,
                (value == 0) ? BlackT::TColor::fullAlphaTransparency
                  : BlackT::TColor::fullAlphaOpacity);
}

int NftrChar::colorToValue(const BlackT::TColor& color, int bpp) {
  // handle transparency
  if (color.a() != TColor::fullAlphaOpacity) return 0;
  
  // un-invert value
  int amt = (0xFF - color.r());
  // divide by level per pixel
  amt /= (0xFF / ((0x01 << bpp) - 1));
  return amt;
}


} 
