#ifndef NCLRFILE_H
#define NCLRFILE_H


#include "ds/NitroHeader.h"
#include "ds/PlttBlock.h"

namespace Nftred {


class NclrFile {
public:
  
  NclrFile();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  NitroHeader header;
  PlttBlock plttBlock;
  
protected:
  
};


}


#endif
