#include "ds/NscrFile.h"
#include "ds/NitroTile.h"
#include "util/TGraphic.h"
#include "util/TStringConversion.h"
#include "util/ByteConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TPngConversion.h"
#include <string>
#include <iostream>

using namespace BlackT;

namespace Nftred {


NscrFile::NscrFile() { }
  
void NscrFile::read(BlackT::TStream& ifs) {
  header.read(ifs);
  scrnBlock.read(ifs);
}

void NscrFile::write(BlackT::TStream& ofs) const {
  int startpos = 0;
  
  header.write(ofs);
  scrnBlock.write(ofs);
  
  // write filesize to header
  int endpos = ofs.tell();
  ofs.seek(startpos + 8);
  ofs.writeu32le(endpos - startpos);
  ofs.seek(endpos);
}
  
void NscrFile::saveToEditable(const char* prefix,
                      const NitroTileSet& tiles,
                      const NitroPalette& palette,
                      bool rotscale) const {
  std::string paletteName = std::string(prefix);
  std::string mapName = std::string(prefix) + "-dat.bin";
  std::string grpName = std::string(prefix) + "-grp.png";
  
  TBufStream ofs(0x100000);
  ofs.writeu32le(rotscale ? 1 : 0);
  write(ofs);
  ofs.save(mapName.c_str());
  
  palette.saveToEditable(paletteName.c_str());
  
  TGraphic g;
  g.resize(scrnBlock.width, scrnBlock.height);
  g.clearTransparent();
  int tilesW = scrnBlock.width / NitroTile::width;
  int tilesH = scrnBlock.height / NitroTile::height;
  int numTiles = tilesW * tilesH;
  
  int pos = 0;
//  for (int j = 0; j < tilesH; j++) {
  for (int i = 0; i < numTiles; i++) {
//    for (int i = 0; i < tilesW; i++) {
      int baseX = 0;
      int baseY = 0;
      
      // screen 3
      if (i >= 0xC00) {
        baseX = 32;
        baseY = 32;
      }
      // screen 2
      else if (i >= 0x800) {
        if (tilesH > tilesW) {
          // shouldn't be reachable
          baseX = 32;
        }
        else {
          baseY = 32;
        }
      }
      // screen 1
      else if (i >= 0x400) {
        if (tilesH > tilesW) {
          baseY = 32;
        }
        else {
          baseX = 32;
        }
      }
      
      int x = baseX + ((i) % 32);
      int y = baseY + (((i) % 0x400) / 32);
      
      int raw;
      int tileindex;
      bool hflip = false;
      bool vflip = false;
//      int palindex = 0;
      if (rotscale) {
        raw = ByteConversion::fromBytes(scrnBlock.data.data() + pos, 1,
          EndiannessTypes::little, SignednessTypes::nosign);
        pos += 1;
        tileindex = raw & 0xFF;
      }
      else {
        raw = ByteConversion::fromBytes(scrnBlock.data.data() + pos, 2,
          EndiannessTypes::little, SignednessTypes::nosign);
        pos += 2;
        tileindex = raw & 0x03FF;
        hflip = (raw & 0x0400) != 0;
        vflip = (raw & 0x0800) != 0;
//        palindex = (raw & 0xF000) >> 12;
      }
      
      if (tileindex >= tiles.numTiles()) continue;
      
      const NitroTile& tile = tiles.tile(tileindex);
      
      TGraphic tileg(NitroTile::width, NitroTile::height);
      tileg.clearTransparent();
      tile.toGraphicPalettized(tileg, palette, 0, 0);
      
      if (hflip) tileg.flipHorizontal();
      if (vflip) tileg.flipVertical();
      
      g.copy(tileg,
             TRect(x * NitroTile::width,
                  y * NitroTile::height,
                  0, 0),
             TRect(0, 0, 0, 0));
//    }
//  }
  }
  
  TPngConversion::graphicToRGBAPng(grpName, g);
}

void NscrFile::patchTileSetFromEditable(const char* prefix,
                              NitroTileSet& tiles) {
  std::string paletteName = std::string(prefix);
  std::string mapName = std::string(prefix) + "-dat.bin";
  std::string grpName = std::string(prefix) + "-grp.png";
  
  TIfstream ifs(mapName.c_str(), std::ios_base::binary);
  bool rotscale = ((ifs.readu32le() == 0) ? false : true);
  read(ifs);
  
  NitroPalette palette;
  palette.loadFromEditable(paletteName.c_str());
  
  TGraphic g;
  TPngConversion::RGBAPngToGraphic(grpName, g);
  int tilesW = g.w() / NitroTile::width;
  int tilesH = g.h() / NitroTile::height;
  int numTiles = tilesW * tilesH;
  
  int pos = 0;
//  for (int j = 0; j < tilesH; j++) {
  for (int i = 0; i < numTiles; i++) {
//    for (int i = 0; i < tilesW; i++) {
      int baseX = 0;
      int baseY = 0;
      
      // screen 3
      if (i >= 0xC00) {
        baseX = 32;
        baseY = 32;
      }
      // screen 2
      else if (i >= 0x800) {
        if (tilesH > tilesW) {
          // shouldn't be reachable
          baseX = 32;
        }
        else {
          baseY = 32;
        }
      }
      // screen 1
      else if (i >= 0x400) {
        if (tilesH > tilesW) {
          baseY = 32;
        }
        else {
          baseX = 32;
        }
      }
      
      int x = baseX + ((i) % 32);
      int y = baseY + (((i) % 0x400) / 32);
      
      int raw;
      int tileindex;
      bool hflip = false;
      bool vflip = false;
//      int palindex = 0;
      if (rotscale) {
        raw = ByteConversion::fromBytes(scrnBlock.data.data() + pos, 1,
          EndiannessTypes::little, SignednessTypes::nosign);
        pos += 1;
        tileindex = raw & 0xFF;
      }
      else {
        raw = ByteConversion::fromBytes(scrnBlock.data.data() + pos, 2,
          EndiannessTypes::little, SignednessTypes::nosign);
        pos += 2;
        tileindex = raw & 0x03FF;
        hflip = (raw & 0x0400) != 0;
        vflip = (raw & 0x0800) != 0;
//        palindex = (raw & 0xF000) >> 12;
      }
      
//      if (tileindex == 0) {
//        std::cout << x << " " << y << std::endl;
//      }
      
//      std::cout << std::hex << tileindex << std::endl;
      
//      if (tileindex >= tiles.numTiles()) continue;
//      if (hflip || vflip) continue;
      
      // allow new tiles to be inserted
      if (tileindex >= tiles.numTiles()) {
        tiles.resize(tileindex + 1);
      }
      
      TGraphic tileg;
      tileg.resize(NitroTile::width, NitroTile::height);
      tileg.clearTransparent();
      tileg.copy(g,
                 TRect(0, 0, 0, 0),
                 TRect(x * NitroTile::width, y * NitroTile::height, 0, 0));
      if (hflip) tileg.flipHorizontal();
      if (vflip) tileg.flipVertical();
      
      NitroTile& tile = tiles.tile(tileindex);
      tile.fromGraphicPalettized(tileg, palette, 0, 0);
//    }
//  }
  }
}


} 
