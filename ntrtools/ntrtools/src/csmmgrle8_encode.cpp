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

int findNextRunLength(const TByte* input, int remaining) {
  TByte c = *input;
  int len = 1;
  while ((len < remaining) && (len < maxRepeatCount) && (input[len] == c))
    ++len;
  
  return len;
}

void csmmgRle8Encode(const TArray<TByte>& input, TOfstream& ofs) {
  int pos = 0;
  int absoluteCount = 0;
  while (true) {
    int repeatLen = findNextRunLength(input.data() + pos,
                                        input.size() - pos);
    
    // case 1a: repeat is most efficient
    // case 1b: we've reached the maximum encodeable absolute run length
    // case 1c: out of space
    if ((repeatLen >= minRepeatLen)
        || (absoluteCount >= maxAbsoluteCount)
        || (pos >= input.size())) {
      // add absolute if it exists
      if (absoluteCount >= minAbsoluteLen) {
        int start = pos - absoluteCount;
        char encodeByte = 0x00;
        encodeByte |= (absoluteCount - minAbsoluteLen);
        ofs.put(encodeByte);
        ofs.write((char*)(input.data() + start),
                  absoluteCount);
      }
      
      // add repeat if it exists
      if (repeatLen >= minRepeatLen) {
        char encodeByte = 0x80;
        encodeByte |= (repeatLen - minRepeatLen);
        ofs.put(encodeByte);
        ofs.put((char)(input[pos]));
        pos += repeatLen;
      }
      
      absoluteCount = 0;
      
      if (pos >= input.size()) break;
    }
    // case 2: repeat is not efficient
    else {
      ++absoluteCount;
      ++pos;
    }
    
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Chou Soujuu Mecha MG RLE8 encoder" << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile>" << endl;
    
    return 0;
  }
  
  char* infile = argv[1];
  char* outfile = argv[2];
  
  TIfstream ifs(infile, ios_base::binary);
  TArray<TByte> input;
  input.resize(ifs.size());
  ifs.read((char*)(input.data()), input.size());
  ifs.close();
  
  TOfstream ofs(outfile, ios_base::binary);
  ofs.writeu8le(0x30);
  ofs.writeInt(input.size(),
    3, EndiannessTypes::little, SignednessTypes::nosign);
  
  csmmgRle8Encode(input, ofs);
  
  return 0;
}
