#ifndef SCRNBLOCK_H
#define SCRNBLOCK_H


#include "util/TStringConversion.h"
#include "util/TStream.h"
#include "util/TByte.h"
#include "util/TArray.h"
#include <vector>

namespace Nftred {


class ScrnBlock {
public:
  ScrnBlock();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;

  char signature[4];
  int size;
  int width;
  int height;
  int internalSize;
  int bgType;
  
  BlackT::TArray<BlackT::TByte> data;
  
  void saveToEditable(const char* prefix) const;
  void loadFromEditable(const char* prefix);
  
};


}


#endif
