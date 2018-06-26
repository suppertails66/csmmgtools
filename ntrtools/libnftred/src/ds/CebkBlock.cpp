#include "ds/CebkBlock.h"
#include "util/ByteConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TPngConversion.h"
#include <iostream>
#include <string>

using namespace BlackT;

namespace Nftred {


void CebkBankEntry::saveAsEditable(const char* prefix,
                                   const NitroTileSet& tiles,
                                   const NitroPalette& palette,
                                   int tileMultiplier,
                                   int start,
                                   int end) const {
  if ((end == -1) || (end >= oams.size())) end = oams.size();
  if ((start == -1) || (start >= oams.size())) start = 0;
  
  std::string imageName = std::string(prefix) + "-img.png";
  std::string bindatName = std::string(prefix) + "-bin.bin";
//  std::string paletteName = std::string(prefix) + "-pal.bin";
//  std::string paletteImageName = std::string(prefix) + "-pal.png";
  
  TGraphic g;
  BlackT::TRect dimensions
    = composeToGraphic(g, tiles, palette, tileMultiplier, start, end);
  TPngConversion::graphicToRGBAPng(imageName, g);
  
  palette.saveToEditable(prefix);
//  TOfstream palofs(paletteName.c_str(), std::ios_base::binary);
//  palette.write(palofs);
  
//  TGraphic palpreview;
//  palette.generatePreview(palpreview);
//  TPngConversion::graphicToRGBAPng(paletteImageName, palpreview);
  
  TOfstream binofs(bindatName.c_str(), std::ios_base::binary);
  binofs.writeu32le(tileMultiplier);
  binofs.writeu32le(dimensions.x());
  binofs.writeu32le(dimensions.y());
  binofs.writeu32le(dimensions.w());
  binofs.writeu32le(dimensions.h());
  binofs.writeu32le(oams.size());
  for (int i = start; i < end; i++) {
    BlackT::TByte buffer[OamAttributeEntry::size];
    oams[i].toRaw(buffer);
    binofs.write((char*)buffer, OamAttributeEntry::size);
  }
  
}

void CebkBankEntry::patchTileSet(const char* prefix,
                  NitroTileSet& tiles) {
  std::string imageName = std::string(prefix) + "-img.png";
  std::string bindatName = std::string(prefix) + "-bin.bin";
//  std::string paletteName = std::string(prefix) + "-pal.bin";
//  std::string paletteImageName = std::string(prefix) + "-pal.png";
  
  TGraphic g;
  TPngConversion::RGBAPngToGraphic(imageName, g);
  
//  NitroPalette palette;
//  TIfstream palifs(paletteName.c_str(), std::ios_base::binary);
//  palette.read(palifs);
  
  NitroPalette palette;
  palette.loadFromEditable(prefix);
  
  TIfstream binifs(bindatName.c_str(), std::ios_base::binary);
  int tileMultiplier = binifs.readu32le();
  int dimx = binifs.readu32le();
  int dimy = binifs.readu32le();
  int dimw = binifs.readu32le();
  int dimh = binifs.readu32le();
  TRect dimensions(dimx, dimy, dimw, dimh);
  int oamsSize = binifs.readu32le();
  std::vector<OamAttributeEntry> oam;
  oam.resize(oamsSize);
  for (int i = 0; i < oamsSize; i++) {
    BlackT::TByte buffer[OamAttributeEntry::size];
    binifs.read((char*)buffer, OamAttributeEntry::size);
    oam[i].fromRaw(buffer);
  }
  
  for (int i = 0; i < oam.size(); i++) {
    // skip OAMs with hflip or vflip applied -- they aren't currently
    // handled, and the flipped tiles are probably used unflipped elsewhere
    if (!(oam[i].rotscale)
        && ((oam[i].vflip)
            || (oam[i].hflip))) {
      continue;
    }
  
    int tileindex = oam[i].tileindex * tileMultiplier;
    int basex = oam[i].xpos + dimensions.x();
    int basey = oam[i].ypos + dimensions.y();
    int oamw = oam[i].width();
    int oamh = oam[i].height();
    
    for (int k = 0; k < oamh; k += NitroTile::height) {
      if (tileindex >= tiles.numTiles()) break;
      for (int j = 0; j < oamw; j += NitroTile::width) {
        if (tileindex >= tiles.numTiles()) break;
        NitroTile tile;
        tile.fromGraphicPalettized(g, palette, basex + j, basey + k);
        tiles.tile(tileindex++) = tile;
      }
    }
  }
}
  
BlackT::TRect CebkBankEntry::composeToGraphic(BlackT::TGraphic& dst,
                      const NitroTileSet& tiles,
                      const NitroPalette& palette,
                      int tileMultiplier,
                      int start,
                      int end) const {
  if ((end == -1) || (end >= oams.size())) end = oams.size();
  if ((start == -1) || (start >= oams.size())) start = 0;
                      
  BlackT::TRect dimensions = computeDimensions();
  dst.resize(dimensions.w(), dimensions.h());
  dst.clearTransparent();
  for (int i = start; i < end; i++) {
    TGraphic obj;
    oams[i].composeToGraphic(obj, tiles, palette, tileMultiplier);
    int x = oams[i].xpos + dimensions.x();
    int y = oams[i].ypos + dimensions.y();
    dst.blit(obj,
             TRect(x, y, 0, 0),
             TRect(0, 0, 0, 0));
  }
  
  return dimensions;
}
                      
BlackT::TRect CebkBankEntry::computeDimensions() const {
  int xlow = 0;
  int xhigh = 0;
  int ylow = 0;
  int yhigh = 0;
  
  for (int i = 0; i < oams.size(); i++) {
    const OamAttributeEntry& entry = oams[i];
    int w = entry.width();
    int h = entry.height();
    
    int left = entry.xpos;
    int right = entry.xpos + w;
    int top = entry.ypos;
    int bottom = entry.ypos + h;
    
    if (left < xlow) xlow = left;
    if (right > xhigh) xhigh = right;
    if (top < ylow) ylow = top;
    if (bottom > yhigh) yhigh = bottom;
  }
  
  // return a rect where x/y is the centerpoint and w/h are the dimensions
  // required to hold all the OBJs
  return TRect(-xlow, -ylow, xhigh - xlow, yhigh - ylow);
}

CebkBlock::CebkBlock()
  : signature { 'C', 'E', 'B', 'K' },
    size(0),
    extended(false),
    tileIndexOffsetFlags(0),
    subImages(false),
    partitionOffset(0) { }
  
void CebkBlock::read(BlackT::TStream& ifs) {
  int startpos = ifs.tell();
  
  ifs.readRev(signature, sizeof(signature));
  size = ifs.readu32le();
  int numBanks = ifs.readu16le();
  extended = (ifs.readu16le() != 0);
  ifs.readu32le();  // data offset (constant, skipped)
  int rawflags = ifs.readu32le();
  tileIndexOffsetFlags = rawflags & 0x0003;
  subImages = rawflags & 0x00040;
  partitionOffset = ifs.readu32le();
  ifs.readu32le();
  ifs.readu32le();
  
  std::vector<CebkBankEntryRaw> rawbanks;
  rawbanks.resize(numBanks);
  for (int i = 0; i < numBanks; i++) {
    rawbanks[i].numOams = ifs.readu16le();
    rawbanks[i].unknown = ifs.readu16le();
    rawbanks[i].oamStartOffset = ifs.readu32le();
    
    if (extended) {
      rawbanks[i].xmax = ifs.readu16le();
      rawbanks[i].ymax = ifs.readu16le();
      rawbanks[i].xmin = ifs.readu16le();
      rawbanks[i].ymin = ifs.readu16le();
    }
  }
  
  int remaining = size - (ifs.tell() - startpos);
  TArray<TByte> oamdata;
  oamdata.resize(remaining);
  ifs.read((char*)(oamdata.data()), remaining);
  
  banks.resize(numBanks);
  for (int i = 0; i < numBanks; i++) {
    banks[i].unknown = rawbanks[i].unknown;
    banks[i].xmax = rawbanks[i].xmax;
    banks[i].ymax = rawbanks[i].xmax;
    banks[i].xmin = rawbanks[i].xmax;
    banks[i].ymin = rawbanks[i].xmax;
  
    int offset = rawbanks[i].oamStartOffset;
    
    banks[i].oams.resize(rawbanks[i].numOams);
    for (int j = 0; j < rawbanks[i].numOams; j++) {
      banks[i].oams[j].fromRaw(
        oamdata.data() + offset + (j * OamAttributeEntry::size));
    }
  }
}

void CebkBlock::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();
  
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeu32le(size);
  ofs.writeu16le(banks.size());
  if (extended) ofs.writeu16le(1);
  else ofs.writeu16le(0);
  ofs.writeu32le(0x18); // data offset
  int rawflags = 0;
  rawflags |= tileIndexOffsetFlags;
  if (subImages) rawflags |= 0x00040;
  ofs.writeu32le(rawflags);
  ofs.writeu32le(partitionOffset);
  ofs.writeu32le(0);
  ofs.writeu32le(0);
  
  // skip index
  int indexpos = ofs.tell();
  ofs.seekoff(banks.size()
    * (extended ? CebkBankEntryRaw::extendedSize : CebkBankEntryRaw::size));
  
  int bankStartOffset = ofs.tell();
  std::vector<int> bankOffsets;
  bankOffsets.resize(banks.size());
  
  for (int i = 0; i < banks.size(); i++) {
    bankOffsets[i] = ofs.tell() - bankStartOffset;
    
    for (int j = 0; j < banks[i].oams.size(); j++) {
      TByte buffer[OamAttributeEntry::size];
      banks[i].oams[j].toRaw(buffer);
      ofs.write((char*)buffer, OamAttributeEntry::size);
    }
  }
  
  ofs.alignToBoundary(4);
  int endpos = ofs.tell();
  
  // write index
  ofs.seek(indexpos);
  for (int i = 0; i < banks.size(); i++) {
    ofs.writeu16le(banks[i].oams.size());
    ofs.writeu16le(banks[i].unknown);
    ofs.writeu32le(bankOffsets[i]);
    
    if (extended) {
      ofs.writeu16le(banks[i].xmax);
      ofs.writeu16le(banks[i].ymax);
      ofs.writeu16le(banks[i].xmin);
      ofs.writeu16le(banks[i].ymin);
    }
  }
  
  ofs.seek(startpos + 4);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}


}
