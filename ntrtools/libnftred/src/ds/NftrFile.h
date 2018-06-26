#ifndef NFTR_FILE_H
#define NFTR_FILE_H


#include "ds/NitroHeader.h"
#include "ds/FinfBlock.h"
#include "ds/CglpBlock.h"
#include "ds/CwdhBlock.h"
#include "ds/CmapBlock.h"
#include <vector>

namespace Nftred {


typedef std::vector<CmapBlock> CmapBlockCollection;
  
class NftrFile {
public:
  friend class NftrFont;

  const static int blockPointerOffset = -8;

  NftrFile();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
protected:
  
  NitroHeader header;
  FinfBlock finfBlock;
  CglpBlock cglpBlock;
  CwdhBlock cwdhBlock;
  CmapBlockCollection cmapBlocks;
  
};


}


#endif
