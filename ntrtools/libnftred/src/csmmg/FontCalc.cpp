#include "csmmg/FontCalc.h"
#include <iostream>

using namespace BlackT;

namespace Nftred {


const static int maxPlayerNameWidth = 70;

const int reducedWidthConversionTable[0x60] = {
  // 0x20: space
  0x20,
  // 0x21-0x27: stuff that should never be mapped
  0, 0, 0, 0, 0, 0, 0,
  // 0x28-0x29: parentheses
  0x28, 0x29,
  // 0x2A-0x2C
  0, 0, 0,
  // 0x2D: hyphen
  0x1D,
  // 0x2E: period
  0x2E,
  // 0x2E-0x2F
  0,
  // 0x30-0x39: digits
//  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x8540, 0x8541, 0x8542, 0x8543, 0x8544,
  0x8545, 0x8546, 0x8547, 0x8548, 0x8549,
  // 0x3A-0x40
  0, 0, 0, 0, 0, 0, 0,
  // 0x41-0x5A: upper case
//  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0x854A, 0x854B, 0x854C, 0x854D, 0x854E,
  0x854F, 0x8550, 0x8551, 0x8552, 0x8553,
  0x8554, 0x8555, 0x8556, 0x8557, 0x8558,
  0xD, 0x8559, 0x855A, 0x855B, 0x855C,
  0x855D, 0x855E, 0x855F, 0x8560, 0x8561,
  0x8562,
  // 0x5B-0x60
  0, 0, 0, 0, 0, 0, 
  // 0x61-0x7A: lower case
//  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0x4, 0x16, 0x10, 0xE, 0x2,
  0x12, 0x15, 0xC, 0x6, 0x1B,
  0x18, 0xF, 0x11, 0x9, 0x5,
  0x7, 0x1A, 0xB, 0x1E, 0x3,
  0x8, 0x17, 0x14, 0x19, 0x13,
  0x1C,
  // 0x7B-0x7F
  0, 0, 0, 0, 0
  
};

int FontCalc::numCharsInString(const std::string& src) {
  int count = 0;
  
  for (int i = 0; i < src.size(); ) {
    int escSize = stringEscapeSeqSize(src, i);
    if (escSize > 0) {
      i += escSize;
      continue;
    }
    
    ++count;
      
    if ((unsigned char)(src[i]) >= 0x80) {
      i += 2;
    }
    else {
      ++i;
    }
  }
  
  return count;
}
  
int FontCalc::nextChar(const std::string& src, int* pos) {
  char next = src[(*pos)++];
  int charcode = ((unsigned int)next & 0xFF);
  // 0x80 or greater = 2-byte sequence
  if ((next < 0) && (*pos < src.size())) {
    next = src[(*pos)++];
    charcode <<= 8;
    charcode |= ((unsigned int)next & 0xFF);
  }
  return charcode;
}
  
void FontCalc::ungetChar(int charcode, int* pos) {
  if (charcode >= 0x100) *pos -= 2;
  else *pos -= 1;
}
  
void FontCalc::appendChar(std::string& dst, int charcode) {
  // 2-byte sequence
  if (charcode >= 0x100) {
    dst += (char)((charcode & 0xFF00) >> 8);
  }
  
  dst += (char)((charcode & 0xFF));
}
  
int FontCalc::escapeSeqSize(char code) {
  switch (code) {
  // newline
  case 'n':
    return 2;
    break;
  // player name
  case 'p':
    return 2;
    break;
  // color change
  case 'c':
    return 3;
    break;
  // ?
  case 'C':
    return 2;
    break;
  // set text speed
  case 'w':
    return 3;
    break;
  // wait
  case 'W':
    return 4;
    break;
  default:
    return 0;
    break;
  }
}
  
int FontCalc::stringEscapeSeqSize(const std::string& src, int pos) {
  int next = nextChar(src, &pos);
  
  // not an escape sequence
  if (next != '/') return 0;
  
  int first = nextChar(src, &pos);
  // can't use 2-byte sequences as an escape sequence
  if (first >= 0x100) return 0;
  return escapeSeqSize(first);
}
  
void FontCalc::preprocess(const std::string& src,
                       std::string& dst) {
  int getpos = 0;
  while (getpos < src.size()) {
    int escSize = stringEscapeSeqSize(src, getpos);
  
    // if this is an escape sequence, copy it
    if (escSize > 0) {
      // append literals
      for (int i = 0; i < escSize; i++) {
        dst += src[getpos++];
      }
    }
    else {
      // read next character
      int next = nextChar(src, &getpos);
      
      // is it something we need to escape?
      if (((unsigned int)next == '.')
          && (getpos <= src.size() - 2)
            && (src[getpos] == '.')
            && (src[getpos + 1] == '.')) {
        // remap the sequence "..." to @ (which in turn is mapped by
        // the font to a precomposed ellipsis)
        appendChar(dst, 0x40);
        
        // addendum: rather than a whole ellipsis, @ now remaps to a period
        // with a slightly larger advance width, so that character-by-character
        // printing looks a bit nicer. so, we now need three @s.
        appendChar(dst, 0x40);
        appendChar(dst, 0x40);
        // actually, make the last one a regular period so we get the right spacing
        // if it's followed by a character
//        appendChar(dst, 0x2E);
        
        getpos += 2;
      }
      else {
        appendChar(dst, next);
      }
      
/*      switch ((unsigned int)next) {
      // newline
//      case ((unsigned int)'\n'):
//        dst += "/n";
//        break;
      // remap the sequence "..." to @ (which in turn is mapped by
      // the font to a precomposed ellipsis)
      case ((unsigned int)'.'):
        if ((getpos < src.size() - 2)
            && (src[getpos + 1] == '.')
            && (src[getpos + 2] == '.')) {
          
        }
        break;
      // standard char
      default:
        appendChar(dst, next);
        break;
      } */
    }
    
  }
  
  // if first character of input is a pipe, convert to reduced-width format
  if ((src.size() > 0) && (src[0] == '|')) {
    getpos = 1;
    
    std::string dstold = dst;
    dst.clear();
    while (getpos < dstold.size()) {
      int escSize = stringEscapeSeqSize(dstold, getpos);
      // if this is an escape sequence, copy it
      if (escSize > 0) {
        // append literals
        for (int i = 0; i < escSize; i++) {
          dst += dstold[getpos++];
        }
      }
      else {
        int next = nextChar(dstold, &getpos);
        
        // non-ascii and newlines can't be converted
        if (((char)next == '\n') || (next < 0x20) || (next >= 0x80)) {
          appendChar(dst, next);
          continue;
        }
        
        int lookupValue = next - 0x20;
        
        int realValue = reducedWidthConversionTable[lookupValue];
        
        appendChar(dst, realValue);
      }
    }
    
/*    for (int i = 0; i < dst.size(); i++) {
      std::cout << std::hex << (unsigned int)((unsigned char)(dst[i])) << " ";
    }
    std::cout << std::endl; */
  }
  // if first character of input is an ampersand, convert to no-x-bearing
  // format
  else if ((src.size() > 0) && (src[0] == '&')) {
    getpos = 1;
    
    std::string dstold = dst;
    dst.clear();
    while (getpos < dstold.size()) {
      int escSize = stringEscapeSeqSize(dstold, getpos);
      // if this is an escape sequence, copy it
      if (escSize > 0) {
        // append literals
        for (int i = 0; i < escSize; i++) {
          dst += dstold[getpos++];
        }
      }
      else {
        int next = nextChar(dstold, &getpos);
        
        // non-ascii and newlines can't be converted
        if (((char)next == '\n') || (next < 0x20) || (next >= 0x80)) {
          appendChar(dst, next);
          continue;
        }
        
        int realValue = next - 0x20 + 0x8563;
        
        appendChar(dst, realValue);
      }
    }
    
  }
}
  
void FontCalc::postprocess(const std::string& src,
                       std::string& dst) {
  int getpos = 0;
  while (getpos < src.size()) {
    int escSize = stringEscapeSeqSize(src, getpos);
  
    // if this is an escape sequence, copy it
    if (escSize > 0) {
      // append literals
      for (int i = 0; i < escSize; i++) {
        dst += src[getpos++];
      }
    }
    else {
      // read next character
      int next = nextChar(src, &getpos);
      
      // is it something we need to escape?
      switch ((unsigned int)next) {
      // newline
      case ((unsigned int)'\n'):
        dst += "/n";
        break;
      // standard char
      default:
        appendChar(dst, next);
        break;
      }
    }
    
  }
}
  
int FontCalc::stringWidth(const std::string& src,
                       const NftrFont& font) {
  int width = 0;
  
  int getpos = 0;
  while (getpos < src.size()) {
    // if this is an escape sequence, skip it
    int escSize = stringEscapeSeqSize(src, getpos);
    if ((escSize > 0) && (getpos < src.size() - 1)
        && (src[getpos + 1] == 'p')) {
      // player name escape sequences obviously contribute to total width
      // player name can be up to 5 characters; we have to assume the
      // worst case of 5 full-width (14px) characters = 70 pixels
      width += maxPlayerNameWidth;
      getpos += escSize;
      continue;
    }
    else if (escSize > 0) {
      getpos += escSize;
      continue;
    }
    
    // get next character
    int charcode = nextChar(src, &getpos);
    
    // look up glyph info (giving up if no glyph is defined for the
    // character code)
    const NftrChar* fontchar = font.getChar(charcode);
    if (fontchar == NULL) return -1;
    
    // add width to total
    width += fontchar->advanceWidth();
  }
  
  return width;
}
  
void FontCalc::nextWord(const std::string& src,
              int* pos,
              std::string& dst) {
//  int start = *pos;
  bool pastInitialSpacing = false;
  while (true) {
    // if we reached the end of the input, we're done
    if (*pos >= src.size()) break;
    
    // if this is an escape sequence, copy it
    // (newlines are escaped in postprocessing, so they aren't captured
    // here)
    int escSize = stringEscapeSeqSize(src, *pos);
    if (escSize > 0) {
      // append literals
      for (int i = 0; i < escSize; i++) {
        dst += src[(*pos)++];
      }
      continue;
    }
    
    // get next character
    int charcode = nextChar(src, pos);
    
    // if a space (ascii or sjis)
    if ((charcode == ' ') || (charcode == 0x8140)) {
      // done
      if (pastInitialSpacing) {
        ungetChar(charcode, pos);
        break;
      }
    }
    // if a newline
    else if (charcode == '\n') {
      // the first time we encounter a newline, unget it and leave it
      // for us to find at the start of the next word
      if (dst.size() > 0) {
        ungetChar(charcode, pos);
      }
      // when we encounter it at the start of the word, return only the
      // newline
      else {
        appendChar(dst, charcode);
      }
      break;
    }
    else {
      // if we got this far, we passed the initial spacing
      pastInitialSpacing = true;
    }
    
    // append to word
    appendChar(dst, charcode);
//    std::cout << std::hex << charcode << " ";
  }
  
//  for (int i = 0; i < dst.size(); i++) {
//    std::cout << std::hex << (int)(dst[i]) << " ";
//  }
//  std::cout << *pos << std::endl;
//  std::cout << src << std::endl;
//  std::cout << std::endl;
}
  
int FontCalc::removeLeadingSpaces(const std::string& src,
                     std::string& dst,
                         const NftrFont& font) {
  int pos = 0;
  int width = 0;
  while (true) {
    // if this is an escape sequence, copy it
    // (newlines are escaped in postprocessing, so they aren't captured
    // here)
/*    int escSize = stringEscapeSeqSize(src, *pos);
    if (escSize > 0) {
      // append literals
      for (int i = 0; i < escSize; i++) {
        dst += src[(*pos)++];
      }
      continue;
    } */
    
    // get next character
    int charcode = nextChar(src, &pos);
    
    // if a space (ascii or sjis)
    if ((charcode == ' ') || (charcode == 0x8140)) {
      const NftrChar* fontchar = font.getChar(charcode);
      if (fontchar == NULL) return -1;
      
      width += fontchar->advanceWidth();
      
      // not done
      continue;
    }
    
    // done once we reach a non-space
    ungetChar(charcode, &pos);
    break;
  }
  
  dst = src.substr(pos, src.size() - pos);
  
  return width;
}
  
int FontCalc::prepString(const std::string& srcraw,
               std::string& dst,
               const NftrFont& font,
               int width,
               int height,
               int rowSpacing) {
  std::string src;
  preprocess(srcraw, src);
               
  int pos = 0;
  
  int x = 0;
  int y = 0;
  while (pos < src.size()) {
/*    char next = src[getpos++];
    int charcode = (unsigned int)next;
    // 0x80 or greater = 2-byte sequence
    if (next < 0) {
      next = src[getpos++];
      charcode <<= 8;
      charcode |= (unsigned int)next;
    } */
    
/*    int escSize = stringEscapeSeqSize(src, getpos);
    // if this is an escape sequence, copy it
    if (escSize > 0) {
      // append literals
      for (int i = 0; i < escSize; i++) {
        dst += src[getpos++];
      }
      continue;
    }
    
    // get next character
    int charcode = nextChar(src, &getpos);
    
    // look up glyph info (giving up if no glyph is defined for the
    // character code)
    const NftrChar* fontchar = font.getChar(charcode);
    if (fontchar == NULL) return -1; */
    
    // error if we exceeded maximum height (e.g. due to a manual newline)
    if ((height > 0) && (y >= height)) return -2;
    
    // get the next word
    std::string nextw;
    nextWord(src, &pos, nextw);
    
//    std::cout << nextw << std::endl;
    
    // Check for manual newline.
    // Since words are broken at newlines, there can only possibly be
    // one newline in the word, and it must be the first character
    // if it exists.
    if ((nextw.size() > 0) && (nextw[0] == '\n')) {
      x = 0;
      y += rowSpacing;
      
      // multiple sequential newlines will be broken up into individual
      // words
//      if (nextw.size() == 1) continue;

      // actually, every newline will be its own word
      dst += '\n';
      continue;
    }
    
    // is there room on the current line for the next word?
    int nextwWidth = stringWidth(nextw, font);
    
    // error if word isn't valid for some reason
    if (nextwWidth == -1) return -3;
    
    // error if word's length exceeds the width of the printing area
    if ((width > 0) && (nextwWidth > width)) return -4;
    
//    std::cout << nextwWidth << " " << std::endl;
    if ((width <= 0) || ((x + nextwWidth) <= width)) {
      // if there is, add it on and continue
      dst += nextw;
      x += nextwWidth;
      continue;
    }
    
    // no room on current line: move to next
    x = 0;
    y += rowSpacing;
    // append newline
    dst += '\n';
    // remove any leading spaces from the next word
    std::string nextwNoSpace;
    int spaceSize = removeLeadingSpaces(nextw, nextwNoSpace, font);
    nextw = nextwNoSpace;
    // remove leading spaces from width count
    nextwWidth -= spaceSize;
    // append content
    dst += nextw;
    // advance x-position
    x += nextwWidth;
    
    // error if we exceeded maximum height
    if ((height > 0) && (nextw.size() > 0) && (y >= height)) return -2;
  }
  
//  std::cout << width << " " << height << std::endl;
  
//  std::cout << std::endl << std::endl;
  
  // postprocess to escape newlines, etc.
  std::string dstraw = dst;
  dst = "";
  postprocess(dstraw, dst);
  
//  std::cout << dst << std::endl << std::endl;
  
  return 0;
}


} 
