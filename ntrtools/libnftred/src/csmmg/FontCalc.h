#ifndef FONTCALC_H
#define FONTCALC_H


#include "ds/NftrFont.h"
#include <string>

namespace Nftred {


class FontCalc {
public:
  
  static int numCharsInString(const std::string& src);
  
  static int nextChar(const std::string& src, int* pos);
  
  static void ungetChar(int charcode, int* pos);
  
  static void appendChar(std::string& dst, int charcode);
  
  static int escapeSeqSize(char code);
  static int stringEscapeSeqSize(const std::string& src, int pos);
  
  static void preprocess(const std::string& src,
                         std::string& dst);
  
  static void postprocess(const std::string& src,
                         std::string& dst);
  
  static int stringWidth(const std::string& src,
                         const NftrFont& font);
  
  static void nextWord(const std::string& src,
                       int* pos,
                       std::string& dst);
  
  static int removeLeadingSpaces(const std::string& src,
                       std::string& dst,
                         const NftrFont& font);
  
  // Prepare an unformatted string for printing by applying word wrap, etc.
  // width and height are the size of the printable area in pixels.
  // A nonzero value is returned if the string can't fit in the given area.
  static int prepString(const std::string& src,
                 std::string& dst,
                 const NftrFont& font,
                 int width,
                 int height,
                 int rowSpacing);
protected:
  
};


}


#endif
