#ifndef CEBKBLOCK_H
#define CEBKBLOCK_H


#include "ds/OamAttributeEntry.h"
#include "util/TArray.h"
#include "util/TStream.h"
#include "util/TByte.h"
#include "util/TCoordPair.h"
#include "util/TRect.h"
#include <vector>

namespace Nftred {


struct CebkBankEntryRaw {
  const static int size = 8;
  const static int extendedSize = 16;

  int numOams;
  int unknown;
  int oamStartOffset;
  
  int xmax;
  int ymax;
  int xmin;
  int ymin;
};

struct CebkBankEntry {
  void saveAsEditable(const char* prefix,
                      const NitroTileSet& tiles,
                      const NitroPalette& palette,
                      int tileMultiplier,
                      int start = 0,
                      int end = -1) const;
  static void patchTileSet(const char* prefix,
                           NitroTileSet& tiles);
  BlackT::TRect composeToGraphic(BlackT::TGraphic& dst,
                        const NitroTileSet& tiles,
                        const NitroPalette& palette,
                        int tileMultiplier,
                        int start = 0,
                        int end = -1) const;
  BlackT::TRect computeDimensions() const;

  int unknown;
  
  int xmax;
  int ymax;
  int xmin;
  int ymin;
  
  std::vector<OamAttributeEntry> oams;
};

// TODO: partitioned data

class CebkBlock {
public:
  CebkBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  char signature[4];
  int size;
//  int numBanks;
  bool extended;
//  int dataOffset;
  int tileIndexOffsetFlags;
  bool subImages;
  int partitionOffset;
  std::vector<CebkBankEntry> banks;
  
protected:
  
};


}


#endif
