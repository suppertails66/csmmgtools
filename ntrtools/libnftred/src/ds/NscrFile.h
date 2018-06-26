#ifndef NSCRFILE_H
#define NSCRFILE_H


#include "ds/NitroHeader.h"
#include "ds/ScrnBlock.h"
#include "ds/NitroTileSet.h"
#include "ds/NitroPalette.h"
#include "util/TArray.h"
#include "util/TStream.h"

namespace Nftred {


class NscrFile {
public:
  
  NscrFile();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  NitroHeader header;
  ScrnBlock scrnBlock;
  
  void saveToEditable(const char* prefix,
                      const NitroTileSet& tiles,
                      const NitroPalette& palette,
                      bool rotscale = false) const;
  void patchTileSetFromEditable(const char* prefix,
                                NitroTileSet& tiles);
//  void loadFromEditable(const char* prefix);
  
protected:
  
};


}


#endif
