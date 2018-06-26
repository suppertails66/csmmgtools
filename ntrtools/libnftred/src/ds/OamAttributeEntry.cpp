#include "ds/OamAttributeEntry.h"
#include "util/ByteConversion.h"
#include <iostream>

using namespace BlackT;

namespace Nftred {


void OamAttributeEntry::composeToGraphic(BlackT::TGraphic& dst,
                        const NitroTileSet& tiles,
                        const NitroPalette& palette,
                        int tileMultiplier) const {
  dst.resize(width(), height());
  
  int tilesX = dst.w() / NitroTile::width;
  int tilesY = dst.h() / NitroTile::height;
  
  int tilepos = tileindex * tileMultiplier;
  
  for (int j = 0; j < tilesY; j++) {
    if (tilepos >= tiles.numTiles()) break;
    
    for (int i = 0; i < tilesX; i++) {
      if (tilepos >= tiles.numTiles()) break;
      
      int x = i * NitroTile::width;
      int y = j * NitroTile::height;
      
      tiles.tile(tilepos++).toGraphicPalettized(dst, palette, x, y);
    }
  }
  
  if (!rotscale) {
    if (hflip) dst.flipHorizontal();
    if (vflip) dst.flipVertical();
  }
}
  
int OamAttributeEntry::width() const {
  switch (objshape) {
  // square
  case 0:
    switch (objsize) {
    case 0:
      return 8;
      break;
    case 1:
      return 16;
      break;
    case 2:
      return 32;
      break;
    case 3:
      return 64;
      break;
    default:
      return -1;
      break;
    }
    break;
  // horizontal
  case 1:
    switch (objsize) {
    case 0:
      return 16;
      break;
    case 1:
      return 32;
      break;
    case 2:
      return 32;
      break;
    case 3:
      return 64;
      break;
    default:
      return -1;
      break;
    }
    break;
  // vertical
  case 2:
    switch (objsize) {
    case 0:
      return 8;
      break;
    case 1:
      return 8;
      break;
    case 2:
      return 16;
      break;
    case 3:
      return 32;
      break;
    default:
      return -1;
      break;
    }
    break;
  default:
    return -1;
    break;
  }
}

int OamAttributeEntry::height() const {
  switch (objshape) {
  // square
  case 0:
    switch (objsize) {
    case 0:
      return 8;
      break;
    case 1:
      return 16;
      break;
    case 2:
      return 32;
      break;
    case 3:
      return 64;
      break;
    default:
      return -1;
      break;
    }
    break;
  // horizontal
  case 1:
    switch (objsize) {
    case 0:
      return 8;
      break;
    case 1:
      return 8;
      break;
    case 2:
      return 16;
      break;
    case 3:
      return 32;
      break;
    default:
      return -1;
      break;
    }
    break;
  // vertical
  case 2:
    switch (objsize) {
    case 0:
      return 16;
      break;
    case 1:
      return 32;
      break;
    case 2:
      return 32;
      break;
    case 3:
      return 64;
      break;
    default:
      return -1;
      break;
    }
    break;
  default:
    return -1;
    break;
  }
}
  
void OamAttributeEntry::fromRaw(const BlackT::TByte* src) {
  int attr0 = ByteConversion::fromBytes(src + 0, 2,
    EndiannessTypes::little, SignednessTypes::nosign);
  int attr1 = ByteConversion::fromBytes(src + 2, 2,
    EndiannessTypes::little, SignednessTypes::nosign);
  int attr2 = ByteConversion::fromBytes(src + 4, 2,
    EndiannessTypes::little, SignednessTypes::nosign);
  
  ypos = (attr0 & 0xFF);
  if (ypos >= 0x80) ypos -= 0x100;
  rotscale = (attr0 & 0x0100);
  if (rotscale) disableobj = (attr0 & 0x0200);
  else doublesize = (attr0 & 0x0200);
  objmode = (attr0 & 0x0C00) >> 10;
  mosaic = (attr0 & 0x1000);
  colordepth = (attr0 & 0x2000) >> 13;
  objshape = (attr0 & 0xC000) >> 14;
  
  xpos = (attr1 & 0x01FF);
  if (xpos >= 0x100) xpos -= 0x200;
  if (!rotscale) {
    hflip = (attr1 & 0x1000);
    vflip = (attr1 & 0x2000);
  }
  else {
    rsparams = (attr1 & 0x3E00) >> 9;
  }
  objsize = (attr1 & 0xC000) >> 14;
  
  tileindex = (attr2 & 0x03FF);
  priority = (attr2 & 0x0C00) >> 10;
  palette = (attr2 & 0xF000) >> 12;
}

void OamAttributeEntry::toRaw(BlackT::TByte* dst) const {
  int attr0 = 0;
  int attr1 = 0;
  int attr2 = 0;
  
  attr0 |= ((ypos < 0) ? (ypos + 0x100) : ypos);
  attr0 |= (rotscale ? 0x0100 : 0x0000);
  if (rotscale) attr0 |= (disableobj ? 0x0200 : 0x0000);
  else attr0 |= (doublesize ? 0x0200 : 0x0000);
  attr0 |= (objmode << 10);
  attr0 |= (mosaic ? 0x1000 : 0x0000);
  attr0 |= (colordepth << 13);
  attr0 |= (objshape << 14);
  
  attr1 |= ((xpos < 0) ? (xpos + 0x200) : xpos);
  if (!rotscale) {
    attr1 |= (hflip ? 0x1000 : 0x0000);
    attr1 |= (vflip ? 0x2000 : 0x0000);
  }
  else {
    attr1 |= (rsparams << 9);
  }
  attr1 |= (objsize << 14);
  
  attr2 |= (tileindex);
  attr2 |= (priority << 10);
  attr2 |= (palette << 12);
  
  ByteConversion::toBytes(attr0, dst + 0, 2,
    EndiannessTypes::little, SignednessTypes::nosign);
  ByteConversion::toBytes(attr1, dst + 2, 2,
    EndiannessTypes::little, SignednessTypes::nosign);
  ByteConversion::toBytes(attr2, dst + 4, 2,
    EndiannessTypes::little, SignednessTypes::nosign);
}


}
