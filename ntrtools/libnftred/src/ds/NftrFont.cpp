#include "ds/NftrFont.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TIniFile.h"
#include "exception/TGenericException.h"
#include <iostream>
#include <fstream>
#include <list>
#include <cstdlib>

using namespace BlackT;

namespace Nftred {


NftrFont::NftrFont() { }
  
const NftrChar* NftrFont::getChar(int codepoint) const {
  CodepointGlyphMap::const_iterator findIt
    = glyphMap_.find(codepoint);
  if (findIt == glyphMap_.cend()) return NULL;
  return findIt->second;
}

void NftrFont::fromNftrFile(const NftrFile& file) {
  const FinfBlock& finfBlock = file.finfBlock;
  const CglpBlock& cglpBlock = file.cglpBlock;
  const CwdhBlock& cwdhBlock = file.cwdhBlock;
  const CmapBlockCollection& cmapBlocks = file.cmapBlocks;
  
  if (cwdhBlock.glyphEntries.size() != cglpBlock.glyphData.size()) {
    throw TGenericException(T_SRCANDLINE,
                            "NftrFont::fromNftrFile()",
                            "CWDH and CGLP size mismatch");
  }
  
  glyphMetricHeight_ = cglpBlock.glyphMetricHeight;
  glyphMetricWidth_ = cglpBlock.glyphMetricWidth;
  bpp_ = cglpBlock.bpp;
  rotation_ = cglpBlock.rotation;
  rawHeight_ = cglpBlock.height;
  rawWidth_ = cglpBlock.width;
  
  height_ = finfBlock.height;
  errorCharIndex_ = finfBlock.errorCharIndex;
  defaultBearingX_ = finfBlock.defaultBearingX;
  defaultWidth_ = finfBlock.defaultWidth;
  defaultAdvanceWidth_ = finfBlock.defaultAdvanceWidth;
  encodingType_ = finfBlock.encodingType;
  
  chars_.resize(cglpBlock.glyphData.size());
  for (int i = 0; i < cglpBlock.glyphData.size(); i++) {
    const CwdhGlyphEntry& cwdhEntry = cwdhBlock.glyphEntries[i];
    TArray<TByte> rawData = cglpBlock.glyphData[i];
  
//    std::cout << "size: " << rawData.size() << std::endl;
  
    NftrChar& c = chars_[i];
    c.fromData(rawData.data(), rawData.size(),
               cglpBlock.width, cglpBlock.height, cglpBlock.bpp,
               cwdhEntry.bearingX, cwdhEntry.width, cwdhEntry.advanceWidth);
  }
  
  
  for (CmapBlockCollection::const_iterator it = cmapBlocks.cbegin();
       it != cmapBlocks.cend();
       ++it) {
    addCmap(*it);
  }
}

void NftrFont::toNftrFile(NftrFile& file) const {
  file.cmapBlocks.clear();
  
  // Write CGLP and CWDH data
  
  file.cglpBlock.glyphData.resize(chars_.size());
  file.cwdhBlock.glyphEntries.resize(chars_.size());
  
  for (int i = 0; i < chars_.size(); i++) {
    const NftrChar& nftrChar = chars_[i];
    int canvasBytes = nftrChar.dataSize(bpp_);
    file.cglpBlock.glyphData[i].resize(canvasBytes);
    nftrChar.toData(file.cglpBlock.glyphData[i].data(), bpp_);
    
    // better hope these are all the same!
    file.cglpBlock.width = nftrChar.pixels.w();
    file.cglpBlock.height = nftrChar.pixels.h();
    file.cglpBlock.glyphSize = canvasBytes;
    file.cglpBlock.glyphMetricHeight = nftrChar.pixels.h();
    file.cglpBlock.glyphMetricWidth = nftrChar.pixels.w();
    
    // fill in CWDH
    file.cwdhBlock.glyphEntries[i].bearingX = nftrChar.bearingX();
    file.cwdhBlock.glyphEntries[i].width = nftrChar.charWidth();
    file.cwdhBlock.glyphEntries[i].advanceWidth = nftrChar.advanceWidth();
    
    // FINF
    file.finfBlock.height = nftrChar.pixels.h();
    
  }
  
//  file.cwdhBlock.firstGlyphCode = glyphMap_.begin()->first;
//  file.cwdhBlock.lastGlyphCode = (--(glyphMap_.end()))->first;
  file.cwdhBlock.firstGlyphCode = 0;
  file.cwdhBlock.lastGlyphCode = chars_.size() - 1;
  
  file.cglpBlock.glyphMetricHeight = glyphMetricHeight_;
  file.cglpBlock.glyphMetricWidth = glyphMetricWidth_;
  file.cglpBlock.bpp = bpp_;
  file.cglpBlock.rotation = rotation_;
  file.cglpBlock.height = rawHeight_;
  file.cglpBlock.width = rawWidth_;
  
  // Write FINF
  
  file.finfBlock.height = height_;
  file.finfBlock.errorCharIndex = errorCharIndex_;
  file.finfBlock.defaultBearingX = defaultBearingX_;
  file.finfBlock.defaultWidth = defaultWidth_;
  file.finfBlock.defaultAdvanceWidth = defaultAdvanceWidth_;
  file.finfBlock.encodingType = encodingType_;
  
  // Generate CMAP blocks for map.
  //
  // CMAP type 0 is used for a contiguous run of codes and glyphs.
  // CMAP type 1 is used for a contiguous run of code mappings to a
  //  non-contiguous sequence of glyphs.
  // CMAP type 2 is used for everything that wouldn't be more efficient
  //  to compress some other way.
  //  Only one of these should be used, and must be placed last (since
  //  it may produce false hits over its full range).
  // 
  // Assuming a type 2 block exists (10 bytes minimum):
  // * Type 0 is efficient for 3+ glyphs
  // * Type 1 is efficient for 5+ glyphs
  //
  // This algorithm currently doesn't take into account the possibility
  // of using 0xFFFF to skip small discontinuities in type 1 chunks.
  
  std::list<CmapType2Entry> cmapType2Entries;
  // character range of type 2 block, produced as we process the items
  // note that this is safe: indices are 16-bit
  int cmap2low = 0x10000;
  int cmap2high = 0x0000;
  
  CodepointGlyphMap::const_iterator it = glyphMap_.begin();
  while (it != glyphMap_.end()) {
    int startCodepoint = it->first;
    
    // get iterator to next non-contiguous codepoint
    CodepointGlyphMap::const_iterator endpos = findNextRunEnd(it);
    
    // get the previous entry (the last codepoint in the contiguous sequence)
    --endpos;
    CodepointGlyphMap::const_iterator endposPrev = endpos;
    ++endpos;
    
    // get number of contiguous codepoints
    int runLength = endposPrev->first - startCodepoint + 1;
    
    // potential type 0
    if ((areGlyphIndicesContiguous(it, endpos))
          && (runLength >= 3)) {
      // add type 0 block
      CmapBlock cmapType0Block;
      cmapType0Block.cmapType = 0;
      cmapType0Block.firstGlyphCode = it->first;
      cmapType0Block.lastGlyphCode = endposPrev->first;
      cmapType0Block.firstCharGlyphNum = getGlyphIndex(it->second);
      file.cmapBlocks.push_back(cmapType0Block);
    }
    // potential type 1
    else {
      if (runLength >= 5) {
        // add type 1 block
        CmapBlock cmapType1Block;
        cmapType1Block.cmapType = 1;
        cmapType1Block.firstGlyphCode = it->first;
        cmapType1Block.lastGlyphCode = endposPrev->first;
        
        while (it != endpos) {
//          std::cout << getGlyphIndex(it->second) << std::endl;
          cmapType1Block.glyphNums.push_back(getGlyphIndex(it->second));
          ++it;
        }
        
        file.cmapBlocks.push_back(cmapType1Block);
      }
      else {
        // add to pending type 2 block
        while (it != endpos) {
          // this strategy of putting all the miscellaneous characters
          // into one big type 2 block doesn't work, for whatever reason --
          // the game freezes
          // maybe it doesn't check the CMAPs in order and gives up after
          // failing to find a character in the declared range?
          CmapType2Entry entry;
          entry.charCode = it->first;
          
          if (it->first < cmap2low) cmap2low = it->first;
          
          if (it->first > cmap2high) cmap2high = it->first;
          
          int glyphIndex = getGlyphIndex(it->second);
          entry.glyphNum = glyphIndex;
          cmapType2Entries.push_back(entry);
          
          
          ++it; 
          
          // this works, but wastes more space than just using a small type 0
/*          CmapBlock cmapType2Block;
          cmapType2Block.cmapType = 2;
          
          CmapType2Entry entry;
          entry.charCode = it->first;
          int glyphIndex = getGlyphIndex(it->second);
          entry.glyphNum = glyphIndex;
          
          cmapType2Block.cmapType2Entries.resize(1);
          cmapType2Block.cmapType2Entries[0] = entry;
          
          cmapType2Block.firstGlyphCode = it->first;
          cmapType2Block.lastGlyphCode = it->first;
          
          file.cmapBlocks.push_back(cmapType2Block);
          
          ++it; */
        }
      }
    }
    
    // move to next run
    it = endpos;
  }
  
  // add type 2 block, if it exists
  if (cmapType2Entries.size() > 0) {
    CmapBlock cmapType2Block;
    cmapType2Block.cmapType = 2;
    
    cmapType2Block.cmapType2Entries.resize(cmapType2Entries.size());
    int cmap2pos = 0;
    for (std::list<CmapType2Entry>::iterator it = cmapType2Entries.begin();
         it != cmapType2Entries.end();
         ++it) {
      cmapType2Block.cmapType2Entries[cmap2pos++] = *it;
    }
    
    cmapType2Block.firstGlyphCode = cmap2low;
    cmapType2Block.lastGlyphCode = cmap2high;
    
    file.cmapBlocks.push_back(cmapType2Block);
  } 
  
  
  
  file.header.numblocks = 3 + file.cmapBlocks.size();
  file.header.version = 0x0100;
  file.header.offset = NitroHeader::size;
}
  
void NftrFont::addFromFont(const NftrFont& font) {
  // what a foolish and amateur mistake I've made
  std::map<int, int> thisShouldBeTheClassMember;
  for (CodepointGlyphMap::const_iterator it = glyphMap_.cbegin();
       it != glyphMap_.cend();
       ++it) {
    thisShouldBeTheClassMember[it->first] = getGlyphIndex(it->second);
  }

  int oldsize = chars_.size();
  chars_.resize(chars_.size() + font.chars_.size());
  for (int i = 0; i < font.chars_.size(); i++) {
//    chars_.push_back(font.chars_[i]);
//    int oldnum = i;
    int newnum = oldsize + i;
    chars_[newnum] = font.chars_[i];
    
    // add any mappings for new glyph
    for (CodepointGlyphMap::const_iterator it = font.glyphMap_.cbegin();
         it != font.glyphMap_.cend();
         ++it) {
       // if glyph pointer is to the current character, add mapping
      if (it->second == &(font.chars_[i])) {
        glyphMap_[it->first] = &(chars_[newnum]);
//        std::cout << it->first << std::endl;
      }
    }
  }
  
  for (std::map<int, int>::const_iterator it
          = thisShouldBeTheClassMember.cbegin();
       it != thisShouldBeTheClassMember.cend();
       ++it) {
    glyphMap_[it->first] = getGlyphPointer(it->second);
  }
  
/*  for (CodepointGlyphMap::const_iterator it = glyphMap_.cbegin();
       it != glyphMap_.cend();
       ++it) {
    std::cout << it->first << " " << getGlyphIndex(it->second) << std::endl;
  } */
}

void NftrFont::addCmap(const CmapBlock& cmap) {
  switch (cmap.cmapType) {
  case 0:
    for (int i = cmap.firstGlyphCode; i <= cmap.lastGlyphCode; i++) {
      glyphMap_[i] = getGlyphPointer(
        cmap.firstCharGlyphNum + (i - cmap.firstGlyphCode));
    }
    break;
  case 1:
    for (int i = cmap.firstGlyphCode; i <= cmap.lastGlyphCode; i++) {
      int index = i - cmap.firstGlyphCode;
      if (cmap.glyphNums[index] == 0xFFFF) continue;
      glyphMap_[i] = getGlyphPointer(cmap.glyphNums[index]);
    }
    break;
  case 2:
    for (int i = 0; i < cmap.cmapType2Entries.size(); i++) {
      glyphMap_[cmap.cmapType2Entries[i].charCode]
        = getGlyphPointer(cmap.cmapType2Entries[i].glyphNum);
    }
    break;
  default:
    throw TGenericException(T_SRCANDLINE,
                            "NftrFont::addCmap(const CmapBlock&)",
                            std::string("Unknown CMAP type: ")
                              + TStringConversion::toString(cmap.cmapType));
    break;
  }
}
  
const NftrChar* NftrFont::getGlyphPointer(int number) const {
  if (number > chars_.size()) {
    throw TGenericException(T_SRCANDLINE,
                            "NftrFont::getGlyphPointer(int) const",
                            std::string("Out-of-range glyph index: ")
                              + TStringConversion::toString(number));
  }
  
  return &(chars_[number]);
}

NftrChar* NftrFont::getGlyphPointer(int number) {
  if (number > chars_.size()) {
    throw TGenericException(T_SRCANDLINE,
                            "NftrFont::getGlyphPointer(int)",
                            std::string("Out-of-range glyph index: ")
                              + TStringConversion::toString(number));
  }
  
  return &(chars_[number]);
}
  
int NftrFont::getGlyphIndex(const NftrChar* glyph) const {
  for (int i = 0; i < chars_.size(); i++) {
    if (&(chars_[i]) == glyph) return i; 
  }
  
/*  throw TGenericException(T_SRCANDLINE,
                          "NftrFont::getGlyphPointerIndex(NftrChar*)",
                          std::string("Couldn't find glyph")); */

  return -1;
}

CodepointGlyphMap::const_iterator NftrFont::findNextRunEnd(
    CodepointGlyphMap::const_iterator it) const {
  if (it == glyphMap_.cend()) return it;
    
//  int lastGlyphIndex = getGlyphIndex(it->second);
  int previousCodepoint = it->first;
  ++it;
  
  while (it != glyphMap_.cend()) {
    if (it->first != (previousCodepoint + 1)) return it;
    
    previousCodepoint = it->first;
    ++it;
  }
  
  return it;
}

bool NftrFont::areGlyphIndicesContiguous(
    CodepointGlyphMap::const_iterator first,
    CodepointGlyphMap::const_iterator second) const {
  if (first == glyphMap_.cend()) return false;
  
  int previous = getGlyphIndex(first->second);
  ++first;
  while (first != second) {
    ++previous;
    
    // make sure we don't go past the glyph limit
    if ((previous >= chars_.size())
        || (getGlyphPointer(previous) != first->second)) {
      return false;
    }
    
    ++first;
  }
  
  return true;
}
  
void NftrFont::save(const char* prefix) const {
//  std::ofstream idx(std::string(prefix) + "nftr_index.txt");
//  std::ofstream cwdh(std::string(prefix) + "nftr_cwdh.txt");
//  std::ofstream cmap(std::string(prefix) + "nftr_map.txt");

  TIniFile idx;
  TIniFile cwdh;
  TIniFile cmap;
  
  idx.setValue("Properties", "rawHeight",
    TStringConversion::intToString(rawHeight_));
  idx.setValue("Properties", "rawWidth",
    TStringConversion::intToString(rawWidth_));
  idx.setValue("Properties", "glyphMetricHeight",
    TStringConversion::intToString(glyphMetricHeight_));
  idx.setValue("Properties", "glyphMetricWidth",
    TStringConversion::intToString(glyphMetricWidth_));
  idx.setValue("Properties", "bpp",
    TStringConversion::intToString(bpp_));
  idx.setValue("Properties", "rotation",
    TStringConversion::intToString(rotation_));
  idx.setValue("Properties", "height",
    TStringConversion::intToString(height_));
  idx.setValue("Properties", "errorCharIndex",
    TStringConversion::intToString(errorCharIndex_));
  idx.setValue("Properties", "defaultBearingX",
    TStringConversion::intToString(defaultBearingX_));
  idx.setValue("Properties", "defaultWidth",
    TStringConversion::intToString(defaultWidth_));
  idx.setValue("Properties", "defaultAdvanceWidth",
    TStringConversion::intToString(defaultAdvanceWidth_));
  idx.setValue("Properties", "encodingType",
    TStringConversion::intToString(encodingType_));
  
  
  for (int i = 0; i < chars_.size(); i++) {
    std::string numstr = TStringConversion::toString(i);
    // lazy lazy lazy
    while (numstr.size() < 5) numstr = "0" + numstr;
  
    TGraphic g;
    chars_[i].toGraphic(g, bpp_);
    TPngConversion::graphicToRGBAPng(
      std::string(prefix) + numstr + ".png",
      g);
    
    cwdh.setValue(numstr, "bearingX",
      TStringConversion::toString(chars_[i].bearingX()));
    cwdh.setValue(numstr, "charWidth",
      TStringConversion::toString(chars_[i].charWidth()));
    cwdh.setValue(numstr, "advanceWidth",
      TStringConversion::toString(chars_[i].advanceWidth()));
//    cwdh << i << " " << chars_[i].bearingX()
//      << chars_[i].
  }
  
  for (CodepointGlyphMap::const_iterator it = glyphMap_.cbegin();
       it != glyphMap_.cend();
       ++it) {
    cmap.setValue("GlyphMap",
                  TStringConversion::intToString(it->first,
                    TStringConversion::baseHex),
                  TStringConversion::intToString(
                    getGlyphIndex(it->second)));
  }
  
  idx.writeFile(std::string(prefix) + "nftr_index.txt");
  cwdh.writeFile(std::string(prefix) + "nftr_cwdh.txt");
  cmap.writeFile(std::string(prefix) + "nftr_cmap.txt");
  
  
}

void NftrFont::load(const char* prefix) {
  TIniFile idx;
  TIniFile cwdh;
  TIniFile cmap;
  
  idx.readFile(std::string(prefix) + "nftr_index.txt");
  cwdh.readFile(std::string(prefix) + "nftr_cwdh.txt");
  cmap.readFile(std::string(prefix) + "nftr_cmap.txt");
  
  rawHeight_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "rawHeight"));
  rawWidth_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "rawWidth"));
  glyphMetricHeight_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "glyphMetricHeight"));
  glyphMetricWidth_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "glyphMetricWidth"));
  bpp_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "bpp"));
  rotation_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "rotation"));
  height_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "height"));
  errorCharIndex_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "errorCharIndex"));
  defaultBearingX_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "defaultBearingX"));
  defaultWidth_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "defaultWidth"));
  defaultAdvanceWidth_ = TStringConversion::stringToInt(
    idx.valueOfKey("Properties", "defaultAdvanceWidth"));
  encodingType_ = static_cast<FinfEncodingTypes::FinfEncodingType>
    (TStringConversion::stringToInt(
      idx.valueOfKey("Properties", "encodingType")));
  
  int numChars = cwdh.numSections();
  chars_.resize(numChars);
  for (int i = 0; i < numChars; i++) {
    std::string numstr = TStringConversion::toString(i);
    // lazy lazy lazy
    while (numstr.size() < 5) numstr = "0" + numstr;
    
    TGraphic g;
    TPngConversion::RGBAPngToGraphic(std::string(prefix) + numstr + ".png",
                                     g);
    chars_[i].fromGraphic(g, bpp_);
    
    chars_[i].setBearingX(TStringConversion::stringToInt(
      cwdh.valueOfKey(numstr, "bearingX")));
    chars_[i].setCharWidth(TStringConversion::stringToInt(
      cwdh.valueOfKey(numstr, "charWidth")));
    chars_[i].setAdvanceWidth(TStringConversion::stringToInt(
      cwdh.valueOfKey(numstr, "advanceWidth")));
  }
  
  glyphMap_.clear();
  for (KeyValueMap::const_iterator it = cmap.sectionCbegin("GlyphMap");
       it != cmap.sectionCend("GlyphMap");
       ++it) {
    int glyphNum = TStringConversion::stringToInt(it->second);
    glyphMap_[TStringConversion::stringToInt(it->first)]
      = getGlyphPointer(glyphNum);
  }
  
}


} 
