#ifndef NCGRTILESET_H
#define NCGRTILESET_H


#include "ds/NcgrFile.h"
#include "ds/NitroTileSet.h"
#include "ds/NitroPalette.h"

namespace Nftred {


class NcgrTileSet {
public:
  NcgrTileSet();
  
  void fromNcgrFile(const NcgrFile& file);
  void toNcgrFile(NcgrFile& file) const;
  
  NitroTileSet tiles_;
  
  // true if the width/height fields came from CHAR's first width/height
  // entries (not FFFF)
  bool whIsPrimary_;
  int height_;
  int width_;
  bool notTiled_;
  bool partitioned_;
  int unknown_;
  
protected:
  
//  NitroPalette grayscale16;
//  NitroPalette grayscale256;
  
};


}


#endif
