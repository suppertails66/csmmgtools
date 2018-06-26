#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TArray.h"
#include <iostream>

using namespace std;
using namespace BlackT;

const static int maxRunLen = 0x7F;
const static int minRepeatLen = 3;
const static int minAbsoluteLen = 1;
const static int maxAbsoluteCount = maxRunLen + minAbsoluteLen;
const static int maxRepeatCount = maxRunLen + minRepeatLen;

void csmmgRle8Decode(const TArray<TByte>& input, TOfstream& ofs) {
  int pos = 0;
  while (pos < input.size()) {
    TByte next = input[pos++];
    
    if (next & 0x80) {
      int count = (int)(next & 0x7F) + minRepeatLen;
      char c = input[pos++];
      for (int i = 0; i < count; i++) ofs.put(c);
    }
    else {
      int count = (int)(next & 0x7F) + minAbsoluteLen;
      for (int i = 0; i < count; i++) ofs.put(input[pos++]);
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Chou Soujuu Mecha MG RLE8 decoder" << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile>" << endl;
    
    return 0;
  }
  
  char* infile = argv[1];
  char* outfile = argv[2];
  
  TIfstream ifs(infile, ios_base::binary);
  TArray<TByte> input;
//  int outsize = ifs.readu32le();
  ifs.readu32le();
  input.resize(ifs.remaining());
  ifs.read((char*)(input.data()), input.size());
  ifs.close();
  
  TOfstream ofs(outfile, ios_base::binary);
  
  csmmgRle8Decode(input, ofs);
  
  return 0;
}
