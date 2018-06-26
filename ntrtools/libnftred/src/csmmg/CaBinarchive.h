#ifndef CABINARCHIVE_H
#define CABINARCHIVE_H


#include "csmmg/CaBinarchiveEntry.h"
#include "util/TArray.h"
#include "util/TByte.h"
#include "util/TStream.h"
#include <vector>

namespace Nftred {


class CaBinarchive {
public:
  
  CaBinarchive();
  
  void readRaw(BlackT::TStream& ifs);
  void writeRaw(BlackT::TStream& ofs) const;
  
  void saveToEditable(const char* prefix) const;
  void loadFromEditable(const char* prefix);
  
  std::vector<CaBinarchiveEntry> entries;
protected:
  
};


}


#endif
