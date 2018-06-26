#include "csmmg/BvmData.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Nftred;

namespace Nftred {


void BvmStringEntry::output(std::ostream& csvofs) const {
  // file, number, offset
  csvofs << filename << ",";
  csvofs << hex << "0x" << number << ",";
  
  csvofs << hex << "0x" << offset << ",";
  
  // japanese text/english placeholder
  csvofs << "\"" << japanese << "\"" << ","
         << "\"" << english << "\"" << ",";
  
  // centering
  csvofs << hex << "0x" << centeringCommands.size();
  for (int i = 0; i < centeringCommands.size(); i++) {
    const CenteringCommand& command = centeringCommands[i];
    
    if (command.hasX && command.hasY) {
      csvofs << hex << " 0x2";
    }
    else {
      csvofs << hex << " 0x1";
    }
    
    if (command.hasX) {
      csvofs << hex << " 0x" << command.offsetOfX;
    }
    
    if (command.hasY) {
      csvofs << hex << " 0x" << command.offsetOfY;
    }
  }
  csvofs << ",";
  
  // references
  csvofs << hex << "0x" << offsetsByte.size();
  for (int i = 0; i < offsetsByte.size(); i++) {
    csvofs << hex << " 0x" << offsetsByte[i];
  }
  csvofs << hex << " 0x" << offsetsWord.size();
  for (int i = 0; i < offsetsWord.size(); i++) {
    csvofs << hex << " 0x" << offsetsWord[i];
  }
  csvofs << ",";
  
  // width/height/lineheight
  csvofs << dec << width << "," << height << "," << lineheight;
  
  csvofs << endl;
}

void escapeString(const std::string& src, std::string& dst) {
  int pos = 0;
  while (pos < src.size()) {
    if ((pos < src.size() - 1) && (src[pos] == '/')) {
      if (src[pos + 1] == 'n') {
        dst += '\n';
        pos += 2;
      }
      else {
        dst += src[pos++];
      }
    }
    else {
      dst += src[pos++];
    }
  }
}

void unescapeString(const std::string& src, std::string& dst) {
  int pos = 0;
  while (pos < src.size()) {
    if ((src[pos] == '\n')) {
      dst += "/n";
      ++pos;
    }
    else {
      dst += src[pos++];
    }
  }
}


}

