#include "ds/NcgrTileSet.h"
#include "exception/TGenericException.h"
#include "util/TStringConversion.h"
#include <iostream>

using namespace std;
using namespace BlackT;

namespace Nftred {


NcgrTileSet::NcgrTileSet()
  : whIsPrimary_(false),
    height_(0),
    width_(0),
    notTiled_(false),
    partitioned_(false),
    unknown_(0) {
  
}

void NcgrTileSet::fromNcgrFile(const NcgrFile& file) {
  int bpp;
  switch (file.charBlock.bpp) {
  case 3:
    bpp = 4;
    break;
  case 4:
    bpp = 8;
    break;
  default:
    throw TGenericException(T_SRCANDLINE,
                            "NcgrTileSet::fromNcgrFile()",
                            "Illegal BPP: "
                              + TStringConversion::intToString(
                                  file.charBlock.bpp));
    break;
  }
  
  tiles_.fromRaw((TByte*)(file.charBlock.data.data()), bpp,
                 file.charBlock.data.size()
                  / (NitroTile::pixelCount / (8 / bpp)));
  
  if ((file.charBlock.width == 0xFFFF)
      || (file.charBlock.height == 0xFFFF)) {
    whIsPrimary_ = false;
    height_ = file.charBlock.height2;
    width_ = file.charBlock.width2;
  }
  else {
    whIsPrimary_ = true;
    height_ = file.charBlock.height;
    width_ = file.charBlock.width;
  }
  
  notTiled_ = file.charBlock.notTiled;
  partitioned_ = file.charBlock.partitioned;
  unknown_ = file.charBlock.unknown;
  
//  cout << file.charBlock.data.size() << endl;
//  cout << tiles_.numTiles() << endl;
}

void NcgrTileSet::toNcgrFile(NcgrFile& file) const {
  file.charBlock.data.resize(tiles_.numTiles()
    * (NitroTile::pixelCount / (8 / tiles_.bpp())));
  tiles_.toRaw((TByte*)(file.charBlock.data.data()));
}


}
