#include "ds/NftrFile.h"
#include <iostream>
#include <string>


namespace Nftred {


NftrFile::NftrFile() {
  
}

void NftrFile::read(BlackT::TStream& ifs) {
  header.read(ifs);
  
  // strictly, we can't depend on FINF being first, but this is good
  // enough for now
  finfBlock.read(ifs, header);
  
  if (finfBlock.cglpOffset != 0) {
    ifs.seek(finfBlock.cglpOffset + blockPointerOffset);
    cglpBlock.read(ifs);
  }
  
  if (finfBlock.cwdhOffset != 0) {
    ifs.seek(finfBlock.cwdhOffset + blockPointerOffset);
    cwdhBlock.read(ifs);
  }
  
//  cout << cglpBlock.width << " " << cglpBlock.height
//    << " " << cglpBlock.glyphMetricHeight
//    << " " << cglpBlock.glyphMetricWidth << endl;

//  cglpBlock.width = 7;
//  cglpBlock.glyphMetricWidth = 8;
//  finfBlock.defaultWidth = 8;
//  finfBlock.defaultAdvanceWidth = 8;
//  for (int i = 0; i < cwdhBlock.glyphEntries.size(); i++) {
//    cout << "Glyph " << cwdhBlock.firstGlyphCode << ": " << endl;
//    cout << "\tbearingX: " << cwdhBlock.glyphEntries[i].bearingX << endl;
//    cout << "\twidth: " << cwdhBlock.glyphEntries[i].width << endl;
//    cout << "\tadvanceWidth: " << cwdhBlock.glyphEntries[i].advanceWidth << endl;
//    cwdhBlock.glyphEntries[i].advanceWidth = 8;
//    cwdhBlock.glyphEntries[i].width = 8;
//  }
  
  if (finfBlock.cmapOffset != 0) {
    int nextpos = finfBlock.cmapOffset + blockPointerOffset;
    int filesize = ifs.size();
    do {
      ifs.seek(nextpos);
      CmapBlock cmapBlock;
      cmapBlock.read(ifs);
      cmapBlocks.push_back(cmapBlock);
      nextpos = cmapBlock.nextBlockOffset + blockPointerOffset;
    } while ((cmapBlocks[cmapBlocks.size() - 1].nextBlockOffset != 0)
             && (cmapBlocks[cmapBlocks.size() - 1].nextBlockOffset
                    < filesize));
  }
  
}

void NftrFile::write(BlackT::TStream& ofs) const {
  int startpos = ofs.tell();

  header.write(ofs);
  
  ofs.alignToBoundary(4);
  int finfAddress = ofs.tell();
  finfBlock.write(ofs, header);
  
  ofs.alignToBoundary(4);
  int cglpAddress = ofs.tell();
  cglpBlock.write(ofs);
  
  ofs.alignToBoundary(4);
  int cwdhAddress = ofs.tell();
  cwdhBlock.write(ofs);
  
  // write cmaps
  ofs.alignToBoundary(4);
  int firstCmapAddress = ofs.tell();
  int previousCmapStart = ofs.tell();
  for (unsigned int i = 0; i < cmapBlocks.size(); i++) {
    ofs.alignToBoundary(4);
    cmapBlocks[i].write(ofs);
    int newCmapStart = ofs.tell();
    
    // write offset of new block to "next" field of previous
    ofs.seek(previousCmapStart + 16);
    ofs.writeu32le(newCmapStart - startpos + 8);
    ofs.seek(newCmapStart);
    
    if (i < cmapBlocks.size() - 1) previousCmapStart = newCmapStart;
  }
  
  // write dummy link value to final cmap
  int endOfLastCmap = ofs.tell();
  ofs.seek(previousCmapStart + 16);
//  ofs.writeu32le(newCmapStart + 8);
  ofs.writeu32le(0);
  ofs.seek(endOfLastCmap);
  
  // write filesize to header
  int endpos = ofs.tell();
  ofs.seek(startpos + 8);
  ofs.writeu32le(endpos - startpos);
//  ofs.seek(endpos);
  
  // write block addresses to finf
  ofs.seek(finfAddress + 16);
  ofs.writeu32le(cglpAddress - startpos + 8);
  ofs.writeu32le(cwdhAddress - startpos + 8);
  ofs.writeu32le(firstCmapAddress - startpos + 8);
  ofs.seek(endpos);
}


} 
