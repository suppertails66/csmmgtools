#ifndef NFTRFONT_H
#define NFTRFONT_H


#include "util/TArray.h"
#include "ds/NftrFile.h"
#include "ds/NftrChar.h"
#include <map>
#include <vector>

namespace Nftred {


typedef std::map<int, const NftrChar*> CodepointGlyphMap;

class NftrFont {
public:
  NftrFont();
  
  const NftrChar* getChar(int codepoint) const;
  
  void fromNftrFile(const NftrFile& file);
  void toNftrFile(NftrFile& file) const;
  
  void addFromFont(const NftrFont& font);
  
//  BlackT::TArray<NftrChar> chars_;
  std::vector<NftrChar> chars_;
  CodepointGlyphMap glyphMap_;
  int rawHeight_;
  int rawWidth_;
  int glyphMetricHeight_;
  int glyphMetricWidth_;
  int bpp_;
  int rotation_;
  
  int height_;
  int errorCharIndex_;
  int defaultBearingX_;
  int defaultWidth_;
  int defaultAdvanceWidth_;
  FinfEncodingTypes::FinfEncodingType encodingType_;
  
  void addCmap(const CmapBlock& cmap);
  
  const NftrChar* getGlyphPointer(int number) const;
  NftrChar* getGlyphPointer(int number);
  
  int getGlyphIndex(const NftrChar* glyph) const;
  
  void save(const char* prefix) const;
  void load(const char* prefix);
protected:
  CodepointGlyphMap::const_iterator findNextRunEnd(
    CodepointGlyphMap::const_iterator it) const;
  bool areGlyphIndicesContiguous(
    CodepointGlyphMap::const_iterator first,
    CodepointGlyphMap::const_iterator second) const;
};


}


#endif
