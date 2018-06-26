#include <iostream>
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TFolderManip.h"
#include "util/TStringConversion.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "ds/NftrFile.h"
#include "ds/NftrFont.h"

using namespace std;
using namespace BlackT;
using namespace Nftred;

int main(int argc, char* argv[]) {
//  TIfstream ifs("fontjp12x12.NFTR", ios_base::binary);
//  NftrFile nftrFile;
//  nftrFile.read(ifs);
  
  NftrFont font;
//  font.fromNftrFile(nftrFile);
  
//  font.save("output/");
  font.load("output/");
//  font.save("output/");
  
  NftrFile nftrFile2;
  font.toNftrFile(nftrFile2);
  TBufStream ofs(0x100000);
  nftrFile2.write(ofs);
  if (ofs.tell() < 0x22224) ofs.seek(0x22224);
  ofs.save("test.NFTR");
  
/*  for (int i = 0; i < font.chars_.size(); i++) {
//  for (int i = 0; i < 8; i++) {
    NftrChar& c = font.chars_[i];
    
    TGraphic g(c.pixels.w(), c.pixels.h());
    for (int k = 0; k < c.pixels.h(); k++) {
      for (int j = 0; j < c.pixels.w(); j++) {
        int value = c.pixels.data(j, k);
//        cout << value << endl;
        int realvalue = (255 - ((value << (9 - font.bpp_)) - 1));
        g.setPixel(j, k, TColor(realvalue, realvalue, realvalue,
          (value == 0) ? TColor::fullAlphaTransparency
            : TColor::fullAlphaOpacity));
      }
    }
    
    TPngConversion::graphicToRGBAPng(string("font_12x12/img-")
               + TStringConversion::toString(i)
               + ".png",
               g);
  } */
  
/*  for (CodepointGlyphMap::iterator it = font.glyphMap_.begin();
       it != font.glyphMap_.end();
       ++it) {
    cout << hex << it->first << " " << hex << font.getGlyphIndex(it->second) << endl;
  } */
  
//  TOfstream ofs("test.NFTR", ios_base::binary);
//  nftrFile.write(ofs);
  
/*  TFolderManip::NameCollection files;
  TFolderManip::getSubFolderListing("./obj", files);
  for (TFolderManip::NameCollection::iterator it = files.begin();
       it != files.end();
       ++it) {
    cout << *it << endl;
  } */
  
  return 0;
}
