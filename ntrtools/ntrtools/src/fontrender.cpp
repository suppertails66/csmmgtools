#include "ds/NftrFile.h"
#include "ds/NftrFont.h"
#include "util/TIfstream.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include <cstring>
#include <vector>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nftred;

int findOpt(int argc, char* argv[], const char* str) {
  for (int i = 0; i < argc - 1; i++) {
    if (strcmp(argv[i], str) == 0) {
      return i;
    }
  }
  
  return -1;
}

char* getOpt(int argc, char* argv[], const char* str) {
  int result = findOpt(argc, argv, str);
  if (result == -1) return NULL;
  return argv[result + 1];
}

TColor rgbToColor(int value) {
  TColor color;
  
  int r = (value & 0xFF0000) >> 16;
  int g = (value & 0x00FF00) >> 8;
  int b = (value & 0x0000FF);
  
  color.setR(r);
  color.setG(g);
  color.setB(b);
  color.setA(TColor::fullAlphaOpacity);
  
  return color;
}

bool isPositionValidForOutline(const TGraphic& g, int x, int y) {
  if ((x < 0) || (x >= g.w()) || (y < 0) || (y >= g.h())) return false;
  
  if (g.getPixel(x, y).a() != TColor::fullAlphaTransparency) return false;
  
  return true;
}

void outlineIfValid(const TGraphic& g, TGraphic& outline, TColor color,
                    int x, int y) {
  if (!isPositionValidForOutline(g, x, y)) return;
  
  outline.setPixel(x, y, color);
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "NFTR font renderer" << endl;
    cout << "Usage: " << argv[0] << " <font> <outfile>" << endl;
    cout << "Options:" << endl;
    cout << "  -t    Set text color (default: 0xFFFFFF)" << endl;
    cout << "  -o    Set outline color (default: no outline)" << endl;
    cout << "The text to render is read from standard input." << endl;
  
    return 0;
  }
  
  TIfstream fontifs(argv[1], ios_base::binary);
  NftrFile nftrFile;
  nftrFile.read(fontifs);
  
  NftrFont font;
  font.fromNftrFile(nftrFile);
  
  bool hasTextColor = false;
  bool hasOutlineColor = false;
  
  char* textColorRaw = getOpt(argc, argv, "-t");
  TColor textColor;
  if (textColorRaw != NULL) {
    hasTextColor = true;
    textColor
      = rgbToColor(TStringConversion::stringToInt(string(textColorRaw)));
  }
  
  char* outlineColorRaw = getOpt(argc, argv, "-o");
  TColor outlineColor;
  if (outlineColorRaw != NULL) {
    hasOutlineColor = true;
    outlineColor
      = rgbToColor(TStringConversion::stringToInt(string(outlineColorRaw)));
  }
  
  vector<int> content;
  
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 32;
  while (cin.good()) {
    unsigned char next = cin.get();
    
    if (!cin.good()) break;
    
    int codepoint = next;
    
    // handle SJIS
    if (next >= 0x80) {
      codepoint <<= 8;
      codepoint |= cin.get();
    }
    
    const NftrChar* nextChar = font.getChar(codepoint);
    if (nextChar == NULL) {
      cerr << "Error: undefined codepoint " << hex << codepoint << endl;
      return -1;
    }
    
    x += nextChar->advanceWidth();
    
    content.push_back(codepoint);
  }
  
  w = x + 16;
  x = 8;
  y = 8;
  
  TGraphic g(w, h);
  g.clearTransparent();
  for (int i = 0; i < content.size(); i++) {
    const NftrChar* nextChar = font.getChar(content[i]);
    nextChar->toGraphic(g, font.bpp_, x + nextChar->bearingX(), y);
    x += nextChar->advanceWidth();
  }
  
  // convert default black color as requested
  if (hasTextColor) {
    for (int j = 0; j < g.h(); j++) {
      for (int i = 0; i < g.w(); i++) {
        if (g.getPixel(i, j).a() != TColor::fullAlphaTransparency) {
          g.setPixel(i, j, textColor);
        }
      }
    }
  }
  
  TGraphic outline(w, h);
  outline.clearTransparent();
  if (hasOutlineColor) {
    for (int j = 0; j < g.h(); j++) {
      for (int i = 0; i < g.w(); i++) {
        if (g.getPixel(i, j).a() != TColor::fullAlphaTransparency) {
          outlineIfValid(g, outline, outlineColor, i - 1, j);
          outlineIfValid(g, outline, outlineColor, i + 1, j);
          outlineIfValid(g, outline, outlineColor, i, j - 1);
          outlineIfValid(g, outline, outlineColor, i, j + 1);
          
          outlineIfValid(g, outline, outlineColor, i - 1, j - 1);
          outlineIfValid(g, outline, outlineColor, i + 1, j - 1);
          outlineIfValid(g, outline, outlineColor, i - 1, j + 1);
          outlineIfValid(g, outline, outlineColor, i + 1, j + 1);
        }
      }
    }
    
    outline.regenerateTransparencyModel();
    
    g.blit(outline,
           TRect(0, 0, 0, 0),
           TRect(0, 0, 0, 0));
  }
  
  TPngConversion::graphicToRGBAPng(string(argv[2]), g);
  
  return 0;
}
