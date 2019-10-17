#ifndef GRAYMANCONSTS_H
#define GRAYMANCONSTS_H


namespace Nftred {


class GrayManConsts {
public:
  const static int code_space   = 0x20;
  const static int code_spaceFullWidth = 0x8140;

  const static int code_digitsLow = 0x824F;
  const static int code_digitsHigh = 0x8258;
  
  const static int code_wait      = 0x81A5;
  const static int code_unk4      = 0x81A3;
  const static int code_br        = 0xA;
  const static int code_end       = 0x00;
  const static int code_unk1      = 0xfffd;
  const static int code_unk2      = 0xfeff;
  const static int code_unk3      = 0xfffc;
};


}


#endif
