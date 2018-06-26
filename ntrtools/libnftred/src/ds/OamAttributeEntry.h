#ifndef OAMATTRIBUTEENTRY_H
#define OAMATTRIBUTEENTRY_H


#include "ds/NitroTileSet.h"
#include "ds/NitroPalette.h"
#include "util/TByte.h"
#include "util/TGraphic.h"

namespace Nftred {


struct OamAttributeEntry {
  const static int size = 6;
  
  void composeToGraphic(BlackT::TGraphic& dst,
                        const NitroTileSet& tiles,
                        const NitroPalette& palette,
                        int tileMultiplier) const;
  int width() const;
  int height() const;

  int ypos;
  int rotscale;
  int disableobj;
  int doublesize;
  int objmode;
  int mosaic;
  int colordepth;
  int objshape;
  
  int xpos;
  int hflip;
  int vflip;
  int rsparams;
  int objsize;
  
  int tileindex;
  int priority;
  int palette;
  
  void fromRaw(const BlackT::TByte* src);
  void toRaw(BlackT::TByte* dst) const;
};


}


#endif
