#ifndef NITROTILESET_H
#define NITROTILESET_H


#include "ds/NitroTile.h"
#include "ds/NitroPalette.h"
#include "util/TArray.h"
#include "util/TStream.h"
#include <vector>

namespace Nftred {


class NitroTileSet {
public:
  NitroTileSet();
  
  int fromRaw(const BlackT::TByte* src, int bpp__, int tileCount);
  int toRaw(BlackT::TByte* dst) const;
  
  void toRawGraphic(BlackT::TGraphic& dst,
                    const NitroPalette& palette) const;
  void fromRawGraphic(const BlackT::TGraphic& src,
                      const NitroPalette& palette,
                      int tileCount);
  
  void saveToEditable(const char* prefix,
                      const NitroPalette& palette) const;
  void loadFromEditable(const char* prefix);
  
  int numTiles() const;
  
  int bpp() const;
  void setBpp(int bpp__);
  
  NitroTile& tile(int index);
  const NitroTile& tile(int index) const;
  
  void addTile(const NitroTile& tile);
  void removeTile(int index);
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  void resize(int size__);
protected:
  const static int rawDumpPerRow = 16;
  
  int bpp_;
  std::vector<NitroTile> tiles_;
};


}


#endif
