#include "util/TFolderManip.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TArray.h"
#include "util/TStringEncoding.h"
#include "util/TStringConversion.h"
#include "util/ByteConversion.h"
#include "util/TCsv.h"
#include "csmmg/OpbData.h"
#include "csmmg/FontCalc.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cstring>

using namespace std;
using namespace BlackT;
using namespace Nftred;

// adding an additional zero to this segfaults on program start
// why??
const int stringTableBufferSize = 0x100000;

void unescapeString(const string& src, string& dst) {
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

int nextIntFromIss(istringstream& iss) {
  string temp;
  iss >> temp;
  return TStringConversion::stringToInt(temp);
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Inserter for Chousoujuu Mecha MG OPB archives" << endl;
    cout << "Usage: " << argv[0] << " <opb> <csv> <font> <outfile>" << endl;
    return 0;
  }
  
  char* opbFilename = argv[1];
  char* csvFilename = argv[2];
  char* fontFilename = argv[3];
  char* outFilename = argv[4];
  
  TIfstream ifs(csvFilename, ios_base::binary);
  TCsv csv;
  csv.readSjis(ifs);
  ifs.close();
  
  ifs.open(opbFilename, ios_base::binary);
  TArray<char> opbFile;
  opbFile.resize(ifs.size());
  ifs.read(opbFile.data(), opbFile.size());
  ifs.close();
  
  ifs.open(fontFilename, ios_base::binary);
  NftrFile nftrFile;
  nftrFile.read(ifs);
  ifs.close();
  NftrFont font;
  font.fromNftrFile(nftrFile);
  
  // start of string table = first entry's offset
  int stringTableStart = TStringConversion::stringToInt(csv.cell(1, 1));
  
  char buffer[stringTableBufferSize];
  int putpos = 0;
  for (int j = 1; j < csv.numRows(); j++) {
    int width = -1;
    int height = -1;
    int lineheight = -1;
    if (csv.numColsInRow(j) >= 8) {
      const std::string& widthstr = csv.cell(5, j);
      const std::string& heightstr = csv.cell(6, j);
      const std::string& lineheightstr = csv.cell(7, j);
      
//      cout << widthstr << " " << heightstr << endl;
      
      if (widthstr.size() > 0)
        width = TStringConversion::stringToInt(widthstr);
      
      if (heightstr.size() > 0)
        height = TStringConversion::stringToInt(heightstr);
      
      if (lineheightstr.size() > 0)
        lineheight = TStringConversion::stringToInt(lineheightstr);
    }
  
    // Write English string to buffer
    const string& engstr = csv.cell(4, j);
    string unescaped;
//    FontCalc::prepString(
//                 const std::string& src,
//                 std::string& dst,
//                 const NftrFont& font,
//                 int width,
//                 int height,
//                 int rowSpacing);
    unescapeString(engstr, unescaped);
    
    const string& jpstr = csv.cell(3, j);
    string unescapedJP;
    unescapeString(jpstr, unescapedJP);
    
    string test;
    int result = FontCalc::prepString(engstr, test,
                                      font,
                                      width,
                                      height,
                                      lineheight);
//    std::cout << test << endl << endl;
    
    std::string resultString;
    if (result == 0) {
      resultString = test;
//      resultString = unescapedJP;
    }
    else {
      resultString = unescapedJP;
      // the japanese version sometimes technically "overflows" the boundaries
      // with characters that don't take up the full width, so ignore any
      // errors in cases where we're inserting the original text without
      // changes
      if (unescaped.compare(unescapedJP) != 0) {
        cerr << "Error (" << result << ") in row " << j << ": " << endl << endl
          << engstr << endl << endl;
        cerr << "Gave up at: " << endl << endl
          << test << endl << endl;
        cerr << "---------------------------------------" << endl << endl;
      }
    }
    
    strcpy(buffer + putpos, resultString.c_str());
    
    // Update pointers
    istringstream iss(csv.cell(2, j));
    int numPointers = nextIntFromIss(iss);
    for (int i = 0; i < numPointers; i++) {
      int pointer = nextIntFromIss(iss);
      char* dst = opbFile.data() + pointer;
      ByteConversion::toBytes(putpos, dst, 4,
        EndiannessTypes::little, SignednessTypes::nosign);
    }
    
    putpos += resultString.size() + 1;
  }
  
  TOfstream ofs(outFilename, ios_base::binary);
  // write everything up to the string table
  ofs.write(opbFile.data(), stringTableStart);
  // write the new string table
  ofs.write(buffer, putpos);
  ofs.close();
  
  return 0;
} 
