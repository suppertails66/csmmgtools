#ifndef NCGRFILE_H
#define NCGRFILE_H


#include "ds/NitroHeader.h"
#include "ds/CharBlock.h"
#include "ds/CposBlock.h"
#include "util/TArray.h"
#include "util/TStream.h"

namespace Nftred {


class NcgrFile {
public:
  
  NcgrFile();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  NitroHeader header;
  CharBlock charBlock;
  
  bool hasCposBlock;
  CposBlock cposBlock;
  
protected:
  
};


}


#endif
