#include "csmmg/BvmData.h"
#include "csmmg/FontCalc.h"
#include "ds/NftrFile.h"
#include "ds/NftrFont.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "util/TCsv.h"
#include "util/TArray.h"
#include "util/TByte.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace BlackT;
using namespace Nftred;

const static int bytesPerIndexEntry = 2;

int nextIntFromIss(istringstream& iss) {
  string temp;
  iss >> temp;
  return TStringConversion::stringToInt(temp);
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Chou Soujuu Mecha MG BVM text inserter" << endl;
    cout << "Usage: " << argv[0] << " <inbvm> <outbvm> [inprefix]"
      << " [addfont]" << endl;
    
    return 0;
  }
  
  char* infilename = argv[1];
  
  char* outfilename = argv[2];
  
  string prefix;
  if (argc >= 4) prefix = string(argv[3]);
  
  string fontprefix;
  if (argc >= 5) fontprefix = string(argv[4]);

  TIfstream bvmifs(infilename, ios_base::binary);
  
//  bvmifs.seek(0x1C);
//  int objNamePointer = bvmifs.readu32le();
  
  bvmifs.seek(0x4C);
  int stuffTablePointer = bvmifs.readu32le();
//  bvmifs.seek(stuffTablePointer + 0x24 + 0x04);
//  int fontPointer = bvmifs.readu32le();
//  int fontSize = bvmifs.readu32le();
  
//  TArray<char> bvm;
//  bvm.resize(bvmifs.size());
//  bvmifs.read(bvm.data(), bvm.size());
  
  bvmifs.seek(0x30);
  int codeBlockOffset = bvmifs.readu32le();
//  int codeBlockSize = offset - codeBlockOffset;
  
  TBufStream bvmofs(0x100000);
  bvmifs.seek(0);
  bvmofs.writeFrom(bvmifs, bvmifs.size());
//  bvmofs.write(bvm.data(), bvm.size());
//  bvm.resize(0);
  
//  TArray<char> font;
//  font.resize(fontifs.size());
//  fontifs.read((char*)(font.data()), font.size());
  
  bvmofs.alignToBoundary(0x10);
  int fontpos = bvmofs.tell();
  TIfstream fontifs((prefix + "-font.nftr").c_str(), ios_base::binary);
  
  NftrFile nftrFile;
  NftrFont nftrFont;
  bool fontExists = (fontifs.size() > 0);
  if (fontExists) {
    // read font
    nftrFile.read(fontifs);
    nftrFont.fromNftrFile(nftrFile);
    
    // change all existing advance widths to 14 pixels (to match original
    // spacing before VWF was installed, overriding the game's monospacing
    // with the smaller values actually given in the NFTR)
    for (int i = 0; i < nftrFont.chars_.size(); i++) {
      NftrChar& c = nftrFont.chars_[i];
//      if (c.advanceWidth() == 12) c.setAdvanceWidth(14);
      c.setAdvanceWidth(14);
    }
    
    // prepare to write font
    bvmofs.seek(fontpos);
    
    // if we're adding characters to the font, make the additions
    // and save the updated font
    int fontsize;
    if (fontprefix.size() > 0) {
    
      NftrFont addfont;
      addfont.load(fontprefix.c_str());
      nftrFont.addFromFont(addfont);
      
      NftrFile newNftrFile;
      nftrFont.toNftrFile(newNftrFile);
      
//      TBufStream junk(0x100000);
//      newNftrFile.write(junk);
//      fontsize = junk.tell();
//      bvmofs.write((char*)(junk.data().data()), fontsize);
      
      newNftrFile.write(bvmofs);
      fontsize = bvmofs.tell() - fontpos;
    }
    // otherwise, just copy the old font
    else {
      fontifs.seek(0);
      fontsize = fontifs.size();
      bvmofs.writeFrom(fontifs, fontsize);
    }
    
    // update pointer and size
    int fontend = bvmofs.tell();
    bvmofs.seek(stuffTablePointer + 0x24 + 0x04);
    bvmofs.writeu32le(fontpos);
    bvmofs.writeu32le(fontsize);
    bvmofs.seek(fontend);
  }
  
  bvmofs.alignToBoundary(0x10);
  int stringpos = bvmofs.tell();
  bvmofs.seek(0x38);
  bvmofs.writeu32le(stringpos);
  bvmofs.seek(stringpos);
  
  TIfstream csvifs((prefix + "-text.csv").c_str(), ios_base::binary);
  TCsv csv;
  csv.readSjis(csvifs);
  
  map<int, BvmStringEntry> stringEntries;
  
  int stringnum = 0;
  for (int j = 1; j < csv.numRows(); j++) {
//    int currentPos = bvmofs.tell();
    
    // get printing area dimensions if they exist
  
    int width = -1;
    int height = -1;
    int lineheight = -1;
    if (csv.numColsInRow(j) >= 10) {
      const std::string& widthstr = csv.cell(7, j);
      const std::string& heightstr = csv.cell(8, j);
      const std::string& lineheightstr = csv.cell(9, j);
      
//      cout << widthstr << " " << heightstr << endl;
      
      if (widthstr.size() > 0)
        width = TStringConversion::stringToInt(widthstr);
      
      if (heightstr.size() > 0)
        height = TStringConversion::stringToInt(heightstr);
      
      if (lineheightstr.size() > 0)
        lineheight = TStringConversion::stringToInt(lineheightstr);
    }
    
    // Retrieve the opcode reference lists
    
    istringstream iss(csv.cell(6, j));
    
    vector<int> offsetsByte;
    int numBytePointers = nextIntFromIss(iss);
    for (int i = 0; i < numBytePointers; i++) {
      int pointer = nextIntFromIss(iss);
      offsetsByte.push_back(pointer);
    }
    
    vector<int> offsetsWord;
    int numWordPointers = nextIntFromIss(iss);
    for (int i = 0; i < numWordPointers; i++) {
      int pointer = nextIntFromIss(iss);
      offsetsWord.push_back(pointer);
    }
    
    // Retrieve the centering list
    
    istringstream ceniss(csv.cell(5, j));
    
    vector<int> centerOffsetsX;
    vector<int> centerOffsetsY;
    int numCenterOffsets = nextIntFromIss(ceniss);
    for (int i = 0; i < numCenterOffsets; i++) {
      // numoffsets is always 2 (for now)
//      int numoffsets = 
      nextIntFromIss(ceniss);
      
      int offsetX = nextIntFromIss(ceniss);
      int offsetY = nextIntFromIss(ceniss);
      
      centerOffsetsX.push_back(offsetX);
      centerOffsetsY.push_back(offsetY);
    }
    
    // if there are no references to this entry, we don't need to bother with
    // it
//    if ((offsetsByte.size() == 0) && (offsetsWord.size() == 0)) {
//      continue;
//    }
  
    // Get raw English and Japanese strings
    
    const string& engstr = csv.cell(4, j);
    string unescaped;
    unescapeString(engstr, unescaped);
    
    const string& jpstr = csv.cell(3, j);
    string unescapedJP;
    unescapeString(jpstr, unescapedJP);
    
    // Prep English string for printing (if possible)
    string test;
    int result = -1;
    if (fontExists && (engstr.size() != 0)) {
      result = FontCalc::prepString(engstr, test,
                                        nftrFont,
                                        width,
                                        height,
                                        lineheight);
    }
//    std::cout << test << endl << endl;
    
    std::string resultString;
    // if no english provided, use JP string
    if (engstr.size() == 0) {
      resultString = unescapedJP;
    }
    // if string prep was successful, use prepped string
    else if (result == 0) {
//      std::cout << test << std::endl;
      resultString = test;
//      resultString = unescapedJP;
    }
    // otherwise, print error message and default to japanese message
    else {
      resultString = unescapedJP;
      // the japanese version sometimes technically "overflows" the boundaries
      // with characters that don't take up the full width, so ignore any
      // errors in cases where we're inserting the original text without
      // changes
      if (unescaped.compare(unescapedJP) != 0) {
//        cerr << "Error in row " << j << ": "
//          << engstr << endl << endl;
        cerr << "Error (" << result << ") in row " << j << ": " << endl << endl
          << engstr << endl << endl;
        cerr << "Gave up at: " << endl << endl
          << test << endl << endl;
        cerr << "---------------------------------------" << endl << endl;
      }
    }
    
    vector<CenteringCommand> centeringCommands;
    for (int i = 0; i < centerOffsetsX.size(); i++) {
      centeringCommands.push_back(CenteringCommand 
        { true, centerOffsetsX[i], 0,
          true, centerOffsetsY[i], 0 }
        );
    }
    
    BvmStringEntry entry {
      csv.cell(0, j),
      TStringConversion::stringToInt(csv.cell(1, j)),
      TStringConversion::stringToInt(csv.cell(2, j)),
      unescapedJP,
      resultString,
      centeringCommands,
      offsetsByte,
      offsetsWord,
      width, height, lineheight };
    if (stringEntries.find(entry.number) == stringEntries.end()) {
      stringEntries[entry.number] = entry;
    }
    
    

/*    // Update opcode references to use an index pointer
    
    for (int i = 0; i < offsetsByte.size(); i++) {
      bvmofs.seek(codeBlockOffset + offsetsByte[i]);
      bvmofs.writeu8le(stringnum);
    }
    
    for (int i = 0; i < offsetsWord.size(); i++) {
      bvmofs.seek(codeBlockOffset + offsetsWord[i]);
      bvmofs.writeu16le(stringnum);
    }
    
    // Write index entry
    bvmofs.seek(indexPos + (stringnum * bytesPerIndexEntry));
    bvmofs.writeInt(currentPos - indexPos, bytesPerIndexEntry,
      EndiannessTypes::little, SignednessTypes::nosign);
    
    // Adjust centering offsets (if we have a font to work with)
    
    if (fontExists) {
      int stringWidth = FontCalc::stringWidth(resultString, nftrFont);
      
      // The game's centering is very coarse-grained -- it works in units
      // of 14 pixels to match the original monospace font. I haven't bothered
      // to change it to do something more exact, so the English text will
      // often be off-center by up to 7 pixels in either direction.
      int approximateChars = stringWidth / 14;
      // Get as close to centered as we can by moving to the left/right
      // of the centerpoint (whichever is closer)
      if (stringWidth % 14 >= 8) {
        ++approximateChars;
      }
      
      for (int i = 0; i < centerOffsetsX.size(); i++) {
        bvmofs.seek(codeBlockOffset + centerOffsetsX[i] + 1);
        bvmofs.writeu8le(approximateChars);
      }
    }
    
    // write the string
    bvmofs.seek(currentPos);
    bvmofs.writeCstr(resultString.c_str());
    ++stringnum; */
  }
  
  // skip over index
  int indexPos = bvmofs.tell();
  int numStringEntries = stringEntries.size();
  bvmofs.seekoff(numStringEntries * bytesPerIndexEntry);
  
  for (map<int, BvmStringEntry>::const_iterator it = stringEntries.cbegin();
       it != stringEntries.cend();
       ++it) {
    int currentPos = bvmofs.tell();
    
    const BvmStringEntry& entry = it->second;
    
    int stringnum = entry.number;
    const string& resultString = entry.english;
    const vector<int>& offsetsByte = entry.offsetsByte;
    const vector<int>& offsetsWord = entry.offsetsWord;
    vector<int> centerOffsetsX;
    for (int i = 0; i < entry.centeringCommands.size(); i++) {
      centerOffsetsX.push_back(entry.centeringCommands[i].offsetOfX);
    }
    
    // Update opcode references to use an index pointer
    
    for (int i = 0; i < offsetsByte.size(); i++) {
      bvmofs.seek(codeBlockOffset + offsetsByte[i]);
      bvmofs.writeu8le(stringnum);
    }
    
    for (int i = 0; i < offsetsWord.size(); i++) {
      bvmofs.seek(codeBlockOffset + offsetsWord[i]);
      bvmofs.writeu16le(stringnum);
    }
    
    // Write index entry
    bvmofs.seek(indexPos + (stringnum * bytesPerIndexEntry));
    bvmofs.writeInt(currentPos - indexPos, bytesPerIndexEntry,
      EndiannessTypes::little, SignednessTypes::nosign);
    
    // Adjust centering offsets (if we have a font to work with)
    
    if (fontExists) {
      int stringWidth = FontCalc::stringWidth(resultString, nftrFont);
      
      // The game's centering is very coarse-grained -- it works in units
      // of 14 pixels to match the original monospace font. I haven't bothered
      // to change it to do something more exact, so the English text will
      // often be off-center by up to 7 pixels in either direction.
      int approximateChars = stringWidth / 14;
      // Get as close to centered as we can by moving to the left/right
      // of the centerpoint (whichever is closer)
      if (stringWidth % 14 >= 8) {
        ++approximateChars;
      }
      
      for (int i = 0; i < centerOffsetsX.size(); i++) {
        bvmofs.seek(codeBlockOffset + centerOffsetsX[i] + 1);
        bvmofs.writeu8le(approximateChars);
      }
    }
    
    // write the string
    bvmofs.seek(currentPos);
    bvmofs.writeCstr(resultString.c_str());
    ++stringnum;
  }
  
  bvmofs.alignToBoundary(0x10);
  bvmofs.save(outfilename);
  
/*  ifs.seek(fontPointer);
  TArray<TByte> font;
  font.resize(fontSize);
  ifs.read((char*)(font.data()), fontSize);
  
  ofstream fontofs(prefix + "-font.nftr", ios_base::binary);
  fontofs.write((char*)(font.data()), fontSize); */
  
  return 0;
} 
