#ifndef NCERFILE_H
#define NCERFILE_H


#include "ds/NitroHeader.h"
#include "ds/CebkBlock.h"
#include "ds/LablBlock.h"
#include "ds/UextBlock.h"
#include "util/TStream.h"

namespace Nftred {


class NcerFile {
public:
  
  NcerFile();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  NitroHeader header;
  CebkBlock cebkBlock;
  bool hasLablBlock;
  LablBlock lablBlock;
  bool hasUextBlock;
  UextBlock uextBlock;
  
protected:
  
};


}


#endif
