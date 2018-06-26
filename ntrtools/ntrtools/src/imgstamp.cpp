#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TStringConversion.h"
#include <string>
#include <iostream>
#include <cstring>

using namespace std;
using namespace BlackT;

enum TransferMode {
  transferBlit,
  transferCopy
};

int main(int argc, char* argv[]) {
  if (argc < 6) {
    cout << "Utility to stamp one image onto another" << endl;
    cout << "Usage: " << argv[0] << " <source> <target> <outfile>"
      " [options]" << endl;
    cout << "Options: " << endl;
    cout << "  -b    Blit mode (default)" << endl;
    cout << "  -c    Copy mode" << endl;
    cout << "  -x    Set x-position (default: 0)" << endl;
    cout << "  -y    Set y-position (default: 0)" << endl;
    
    return 0;
  }
  
  TransferMode mode = transferBlit;
  
  int x = 0;
  int y = 0;
  
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-b") == 0) {
      mode = transferBlit;
    }
    else if (strcmp(argv[i], "-c") == 0) {
      mode = transferCopy;
    }
  }
  
  for (int i = 0; i < argc - 1; i++) {
    if (strcmp(argv[i], "-x") == 0) {
      x = TStringConversion::stringToInt(string(argv[i + 1]));
    }
    else if (strcmp(argv[i], "-y") == 0) {
      y = TStringConversion::stringToInt(string(argv[i + 1]));
    }
  }
  
  TGraphic source;
  TPngConversion::RGBAPngToGraphic(string(argv[1]), source);
  
  TGraphic target;
  TPngConversion::RGBAPngToGraphic(string(argv[2]), target);
  
  if (mode == transferBlit) {
    target.blit(source,
                TRect(x, y, 0, 0),
                TRect(0, 0, 0, 0));
  }
  else {
    target.copy(source,
                TRect(x, y, 0, 0),
                TRect(0, 0, 0, 0));
  }
  
  TPngConversion::graphicToRGBAPng(string(argv[3]), target);
  
  return 0;
}
