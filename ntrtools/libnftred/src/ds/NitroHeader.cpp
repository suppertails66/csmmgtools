#include "ds/NitroHeader.h"
#include "util/TSerialize.h"
#include "exception/TGenericException.h"

using namespace BlackT;

namespace Nftred {


NitroHeader::NitroHeader()
  : signature { 'N', 'F', 'T', 'R' },
    bom(0xFEFF),
    version(0x0100),
    filesize(0),
    offset(0x10),
    numblocks(0) {
  
}
  
void NitroHeader::read(BlackT::TStream& ifs) {
  if (ifs.remaining() < size) {
    throw TGenericException(T_SRCANDLINE,
                            "NitroHeader::read(BlackT::TStream&)",
                            "Not enough space in stream for header");
  }

  ifs.readRev(signature, sizeof(signature));
  bom = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  version = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  filesize = ifs.readInt(4, EndiannessTypes::little, SignednessTypes::nosign);
  offset = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
  numblocks = ifs.readInt(2, EndiannessTypes::little, SignednessTypes::nosign);
}

void NitroHeader::write(BlackT::TStream& ofs) const {
  ofs.writeRev(signature, sizeof(signature));
  ofs.writeInt(bom, 2, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(version, 2, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(filesize, 4, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(offset, 2, EndiannessTypes::little, SignednessTypes::nosign);
  ofs.writeInt(numblocks, 2, EndiannessTypes::little, SignednessTypes::nosign);
}


} 
