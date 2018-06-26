#include "csmmg/BvmData.h"
#include "csmmg/FontCalc.h"
#include "util/TStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TArray.h"
#include "util/TByte.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace std;
using namespace BlackT;
using namespace Nftred;

// Stop searching for centering commands associated with a string reference
// after this many bytes
const static int centeringSearchLimit = 16;

CenteringCommand getCenteringCommand(const TArray<TByte>& codeBlock,
                                     int startingOffset,
                                     const std::string& str) {
  CenteringCommand result { false, -1, -1, false, -1, -1 };
  
  int strSize = FontCalc::numCharsInString(str);
  
//  cout << str << endl;
//  cout << strSize << endl;
  
  int limit = startingOffset + centeringSearchLimit;
  if (limit > codeBlock.size() - 2) limit = codeBlock.size() - 2;
  
  // Search for commands related to text centering.
  // This is set with opcode 0x55, which is also used for other things,
  // so we have to be careful.
  //
  // Our heuristic is to search for the sequence 55 XX A9 in the near
  // vicinity of the string reference opcodes. XX must be the length of
  // the current string in characters. If another 55 XX A9 sequence follows
  // (prior to any other string references), it's the number of lines
  // to skip (plus one) -- this should always be 55 02 A9.
  for (int i = startingOffset; i < limit; i++) {
    TByte next = codeBlock[i];
    
    // if we encounter a string reference, we're done
    if ((next == shortStringRefOpcode)
        || (next == longStringRefOpcode)) {
      break;
    }
    
    // potential hit
    if ((next == centeringStartOpcode)
        && (codeBlock[i + 2] == centeringThirdByte)) {
      int length = codeBlock[i + 1];
      
      // if we already have an x-command, search for y-command
      if (result.hasX) {
        // y
        if ((length < minCenteringYValue)
            || (length > maxCenteringYValue)) continue;
        
        result.hasY = true;
        result.offsetOfY = i;
        result.numLines = length;
        
        // if we find both x and y, we're done
        return result;
      }
      // otherwise, search for x-command
      else {
        // x
        if ((length < minCenteringXValue)
            || (length > maxCenteringXValue)) continue;
        
        // nope, sometimes the listed number of characters isn't actually
        // the number of characters in the string
        // were they seriously doing these manually?!
//        if (length != strSize) continue;
        
        result.hasX = true;
        result.offsetOfX = i;
        result.numChars = length;
      }
    }
  }
  
//  return result;
  
  // if we fail to find both x and y, return failure result
  return CenteringCommand { false, -1, -1, false, -1, -1 };
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Chou Soujuu Mecha MG BVM text extractor" << endl;
    cout << "Usage: " << argv[0] << " <bvm> [outprefix]" << endl;
    
    return 0;
  }
  
  char* filename = argv[1];
  
  string prefix;
  if (argc >= 3) prefix = string(argv[2]);

  TIfstream ifs(filename, ios_base::binary);
  
  ifs.seek(0x18);
  int numObjNames = ifs.readu32le();
//  ifs.seek(0x1C);
  int objNamePointer = ifs.readu32le();
  ifs.seek(objNamePointer);
  int objNameStart = ifs.readu32le();
  
  ifs.seek(0x38);
  
//  int numentries = ifs.readu32le();
  int offset = ifs.readu32le();
/*  int size = objNameStart - offset;
  if (numObjNames == 0) {
    ifs.seek(0x3C);
    size = ifs.readu32le() - offset - 2;
  } */
/*  ifs.seek(0x4C);
  int size = ifs.readu32le() - offset;
  ifs.seek(offset + size - 1);
  // skip past any filler after last string
  while ((unsigned char)(ifs.get()) != (unsigned char)(0x00)) {
    ifs.seekoff(-2);
    --size;
  } */
  ifs.seek(0x3C);
  int size = ifs.readu32le() - offset - 2;
  
  ifs.seek(0x30);
  int codeBlockOffset = ifs.readu32le();
  int codeBlockSize = offset - codeBlockOffset;
  TArray<TByte> codeBlock;
  codeBlock.resize(codeBlockSize);
  ifs.seek(codeBlockOffset);
  ifs.read((char*)(codeBlock.data()), codeBlockSize);
  
  ifs.seek(offset);
  
  ofstream csvofs(prefix + "-text.csv", ios_base::binary);
  
  csvofs << "File,Number,Offset,Japanese,English,Centering,References"
            ",width,height,lineheight"
    << endl;
    
  map<int, BvmStringEntry> stringEntries;
  
  int num = 0;
  while ((ifs.tell() - offset) < size) {
    int stringChunkPos = (ifs.tell() - offset);
    
    vector<int> offsetsByte;
    vector<int> offsetsWord;
    for (int i = 0; i < codeBlock.size(); ) {
      int remaining = codeBlock.size() - i;
      
      // pushByte?
      if ((stringChunkPos <= 0xFF) && (remaining >= 2)
          && (codeBlock[i] == shortStringRefOpcode)
          && (codeBlock[i + 1] == stringChunkPos)) {
        offsetsByte.push_back(i + 1);
        i += 2;
      }
      // pushWord?
      else if ((remaining >= 3) && (codeBlock[i] == longStringRefOpcode)) {
        int off = ByteConversion::fromBytes(codeBlock.data() + i + 1, 2,
                    EndiannessTypes::little, SignednessTypes::nosign);
        if (off == stringChunkPos) {
          offsetsWord.push_back(i + 1);
          i += 3;
        }
        else {
          ++i;
        }
      }
      else {
        ++i;
      }
    }
    
    string str;
    ifs.readCstrString(str);
//    cout << str << endl << endl;
    
    // Find centering commands, if they exist
    vector<CenteringCommand> centeringCommands;
    for (int i = 0; i < offsetsByte.size(); i++) {
      CenteringCommand centerCommand = getCenteringCommand(codeBlock,
                                                        offsetsByte[i] + 1,
                                                        str);
      
      if (centerCommand.hasX) centeringCommands.push_back(centerCommand);
    }
    for (int i = 0; i < offsetsWord.size(); i++) {
      CenteringCommand centerCommand = getCenteringCommand(codeBlock,
                                                        offsetsWord[i] + 2,
                                                        str);
      
      if (centerCommand.hasX) centeringCommands.push_back(centerCommand);
    }
    
    string escaped;
    escapeString(str, escaped);
    
    BvmStringEntry entry
      { filename, num, stringChunkPos,
        escaped, "", centeringCommands,
        offsetsByte, offsetsWord,
        196, 42, 14 };
    
    stringEntries[num] = entry;
        
    ++num;

    // so here's how it works:
    // rather than reading sequentially or using an index, strings are
    // directly referenced in the game scripts by their position relative
    // to the start of the string table. this is very bad for us.
    // to avoid having to reassemble the scripts, we can do the following:
    // 1. heuristically find references to strings within the scripts.
    //    it looks like these are always referenced using ops 5a (1-byte)
    //    and 9a (2-byte), which gives us a reasonably strong guessing method.
    //    track these (and the reference sizes) in the output csv.
    // 2. ensure that we will never (probably) have to change a 1-byte
    //    opcode to a 2-byte opcode. oh wait we can't do this, fuck.
    
    // file, number, offset
/*    csvofs << filename << ",";
    csvofs << hex << "0x" << num++ << ",";
    
    csvofs << hex << "0x" << stringChunkPos << ",";
    
    // japanese text/english placeholder
    csvofs << "\"" << escaped << "\"" << "," << ",";
    
    // centering
    csvofs << hex << "0x" << centeringCommands.size();
    for (int i = 0; i < centeringCommands.size(); i++) {
      CenteringCommand& command = centeringCommands[i];
      
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
    csvofs << "196,42,14";
    
    csvofs << endl; */
  }
  
  // Associate every string reference with its corresponding string.
  // This results in duplicate entries where strings were recycled,
  // but that's OK: it makes the script more readable by indicating
  // characters, portraits, etc.
  //
  // This has the side effect of eliminating unreferenced strings
  // -- is that what we actually want?
  // (upon testing: no)
  map<int, BvmStringEntry> entriesByOffset;
  for (map<int, BvmStringEntry>::const_iterator it = stringEntries.cbegin();
       it != stringEntries.cend();
       ++it) {
    const BvmStringEntry& entry = it->second;
    for (int i = 0; i < entry.offsetsByte.size(); i++) {
      entriesByOffset[entry.offsetsByte[i]] = entry;    
    }
    for (int i = 0; i < entry.offsetsWord.size(); i++) {
      entriesByOffset[entry.offsetsWord[i]] = entry;    
    }
  }
  
  // Output all used strings
  map<int, bool> usedStrings;
  for (map<int, BvmStringEntry>::const_iterator it = entriesByOffset.cbegin();
       it != entriesByOffset.cend();
       ++it) {
    it->second.output(csvofs);
    usedStrings[it->second.number] = true;
  }
  
  // Output all "unused" strings
  for (int i = 0; i < stringEntries.size(); i++) {
    if (usedStrings.find(i) == usedStrings.end()) {
      stringEntries[i].output(csvofs);
    }
  }
  
  ifs.seek(0x4C);
  int stuffTablePointer = ifs.readu32le();
  ifs.seek(stuffTablePointer + 0x24 + 0x04);
  int fontPointer = ifs.readu32le();
  int fontSize = ifs.readu32le();
  
//  cout << fontPointer << " " << fontSize << endl;
  
  // extract font only if it exists
  if ((fontPointer >= 0) && (fontPointer < ifs.size())
      && (fontSize > 0) && (fontSize < ifs.size())) {
    ifs.seek(fontPointer);
    char check[4];
    ifs.read(check, 4);
    if (memcmp(check, "RTFN", 4) == 0) {
      ifs.seek(fontPointer);
      TArray<TByte> font;
      font.resize(fontSize);
      ifs.read((char*)(font.data()), fontSize);
      ofstream fontofs(prefix + "-font.nftr", ios_base::binary);
      fontofs.write((char*)(font.data()), fontSize);
    }
  }
  
  return 0;
} 
