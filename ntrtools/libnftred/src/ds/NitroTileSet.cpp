#include "ds/NitroTileSet.h"
#include "exception/TGenericException.h"
#include "util/TStringConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TPngConversion.h"

using namespace BlackT;

namespace Nftred {


NitroTileSet::NitroTileSet()
  : bpp_(0) { }
  
int NitroTileSet::fromRaw(const BlackT::TByte* src, int bpp__, int tileCount) {
  bpp_ = bpp__;
  
  int pos = 0;
  tiles_.resize(tileCount);
  for (int i = 0; i < tileCount; i++) {
    pos += tiles_[i].fromRaw(src + pos, bpp_);
  }
  
  return pos;
}

int NitroTileSet::toRaw(BlackT::TByte* dst) const {
  int pos = 0;
  for (int i = 0; i < tiles_.size(); i++) {
    pos += tiles_[i].toRaw(dst + pos, bpp_);
  }
  
  return pos;
}
  
void NitroTileSet::toRawGraphic(BlackT::TGraphic& dst,
                  const NitroPalette& palette) const {
  int numtile = numTiles();
  int w = rawDumpPerRow * NitroTile::width;
  // account for non-divisible numbers of tiles with an extra row
  int h = ((numtile / rawDumpPerRow) + 1) * NitroTile::height;
  
  dst.resize(w, h);
  dst.clearTransparent();
  
  for (int i = 0; i < tiles_.size(); i++) {
    int x = (i % rawDumpPerRow) * NitroTile::width;
    int y = (i / rawDumpPerRow) * NitroTile::height;
    
    tiles_[i].toGraphicPalettized(dst, palette, x, y);
  }
}

void NitroTileSet::fromRawGraphic(const BlackT::TGraphic& src,
                    const NitroPalette& palette,
                    int tileCount) {
  tiles_.resize(tileCount);
  
  for (int i = 0; i < tiles_.size(); i++) {
    int x = (i % rawDumpPerRow) * NitroTile::width;
    int y = (i / rawDumpPerRow) * NitroTile::height;
    
    tiles_[i].fromGraphicPalettized(src, palette, x, y);
  }
}

void NitroTileSet::saveToEditable(const char* prefix,
                      const NitroPalette& palette) const {
  std::string graphicName = std::string(prefix) + "-grp.png";
  std::string bindatName = std::string(prefix) + "-grp.bin";
  
  palette.saveToEditable(prefix);
  
  TOfstream binofs(bindatName.c_str(), std::ios_base::binary);
  binofs.writeu32le(numTiles());
  
  TGraphic g;
  toRawGraphic(g, palette);
  TPngConversion::graphicToRGBAPng(graphicName, g);
}

void NitroTileSet::loadFromEditable(const char* prefix) {
  std::string graphicName = std::string(prefix) + "-grp.png";
  std::string bindatName = std::string(prefix) + "-grp.bin";
  
  NitroPalette palette;
  palette.loadFromEditable(prefix);
  
  TIfstream binifs(bindatName.c_str(), std::ios_base::binary);
  int tileCount = binifs.readu32le();
  
  TGraphic g;
  TPngConversion::RGBAPngToGraphic(graphicName, g);
  fromRawGraphic(g, palette, tileCount);
}

int NitroTileSet::numTiles() const {
  return tiles_.size();
}

int NitroTileSet::bpp() const {
  return bpp_;
}

void NitroTileSet::setBpp(int bpp__) {
  bpp_ = bpp__;
}

NitroTile& NitroTileSet::tile(int index) {
  if ((unsigned int)index >= tiles_.size()) {
    throw TGenericException(T_SRCANDLINE,
                            "NitroTileSet::tile()",
                            "Out-of-range access:"
                              + TStringConversion::toString(index));
  }
  
  return tiles_[index];
}

const NitroTile& NitroTileSet::tile(int index) const {
  if ((unsigned int)index >= tiles_.size()) {
    throw TGenericException(T_SRCANDLINE,
                            "NitroTileSet::tile() const",
                            "Out-of-range access:"
                              + TStringConversion::toString(index));
  }
  
  return tiles_[index];
}
  
void NitroTileSet::addTile(const NitroTile& tile) {
  tiles_.push_back(tile);
}

void NitroTileSet::removeTile(int index) {
  tiles_.erase(tiles_.begin() + index);
}

void NitroTileSet::read(BlackT::TStream& ifs) {
  
}

void NitroTileSet::write(BlackT::TStream& ofs) const {
  
}
  
void NitroTileSet::resize(int size__) {
  tiles_.resize(size__);
}


} 
