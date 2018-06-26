#include "ds/NcgrFile.h"
#include "ds/NcerFile.h"
#include "ds/NclrFile.h"
#include "ds/NscrFile.h"
#include "ds/NcgrTileSet.h"
#include "ds/NitroPalette.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TPngConversion.h"
#include "util/TStringConversion.h"
#include <iostream>
#include <cstring>
#include <string>

using namespace std;
using namespace BlackT;
using namespace Nftred;
  
NitroPalette grayscale16;
NitroPalette grayscale256;

int findOpt(int argc, char* argv[], const char* str) {
  for (int i = 0; i < argc - 1; i++) {
    if (strcmp(argv[i], str) == 0) {
      return i;
    }
  }
  
  return -1;
}

char* getOpt(int argc, char* argv[], const char* str) {
  int result = findOpt(argc, argv, str);
  if (result == -1) return NULL;
  return argv[result + 1];
}

int extractImages(char* prefix, char* palette, char* tile, char* cell,
                  int palettenum, int tileMultiplier = -1,
                  int splitPoint = -1) {
  if (prefix == NULL) {
    cerr << "Error: missing output prefix" << endl;
    return 1;
  }
  else if (tile == NULL) {
    cerr << "Error: missing input NCGR file" << endl;
    return 1;
  }
  else if (cell == NULL) {
    cerr << "Error: missing input NCER file" << endl;
    return 1;
  }
  
  TIfstream ifs(tile, ios_base::binary);
  NcgrFile ncgr;
  ncgr.read(ifs);
  NcgrTileSet tiles;
  tiles.fromNcgrFile(ncgr);

  TIfstream ifs2(cell, ios_base::binary);
  NcerFile ncer;
  ncer.read(ifs2);
  
  NitroPalette pal = (tiles.tiles_.bpp() == 4)
    ? grayscale16 : grayscale256;
  if (palette != NULL) {
    TIfstream ifs3(palette, ios_base::binary);
    NclrFile nclr;
    nclr.read(ifs3);
    if (palettenum < nclr.plttBlock.palettes.size()) {
      pal = nclr.plttBlock.palettes[palettenum];
    }
    else {
      cout << "Out-of-range palette index " << palettenum
        << " (max " << (nclr.plttBlock.palettes.size() - 1)
        << "); defaulting to grayscale" << endl;
    }
  }
  
  if (tileMultiplier == -1) {
    tileMultiplier = 1;
    if (ncer.cebkBlock.tileIndexOffsetFlags & 0x01) tileMultiplier *= 2;
    if (ncer.cebkBlock.tileIndexOffsetFlags & 0x02) tileMultiplier *= 2;
  }
  
  for (int i = 0; i < ncer.cebkBlock.banks.size(); i++) {
    if (splitPoint == -1) {
      ncer.cebkBlock.banks[i].saveAsEditable((string(prefix)
        + TStringConversion::intToString(i)).c_str(),
            tiles.tiles_, pal,
            tileMultiplier);
    }
    else {
      ncer.cebkBlock.banks[i].saveAsEditable((string(prefix)
        + TStringConversion::intToString(i) + "-a").c_str(),
            tiles.tiles_, pal,
            tileMultiplier,
            0, splitPoint);
      ncer.cebkBlock.banks[i].saveAsEditable((string(prefix)
        + TStringConversion::intToString(i) + "-b").c_str(),
            tiles.tiles_, pal,
            tileMultiplier,
            splitPoint, -1);
    }
  }
  
  return 0;
}

int extractScreen(char* prefix, char* palette, char* tile, char* screen,
                  int palettenum, bool rotscale) {
  if (prefix == NULL) {
    cerr << "Error: missing output prefix" << endl;
    return 1;
  }
  else if (tile == NULL) {
    cerr << "Error: missing input NCGR file" << endl;
    return 1;
  }
  else if (screen == NULL) {
    cerr << "Error: missing input NCER file" << endl;
    return 1;
  }
  
  TIfstream ifs(tile, ios_base::binary);
  NcgrFile ncgr;
  ncgr.read(ifs);
  NcgrTileSet tiles;
  tiles.fromNcgrFile(ncgr);

  TIfstream ifs2(screen, ios_base::binary);
  NscrFile nscr;
  nscr.read(ifs2);
  
  NitroPalette pal = (tiles.tiles_.bpp() == 4)
    ? grayscale16 : grayscale256;
  if (palette != NULL) {
    TIfstream ifs3(palette, ios_base::binary);
    NclrFile nclr;
    nclr.read(ifs3);
    if (palettenum < nclr.plttBlock.palettes.size()) {
      pal = nclr.plttBlock.palettes[palettenum];
    }
    else {
      cout << "Out-of-range palette index " << palettenum
        << " (max " << (nclr.plttBlock.palettes.size() - 1)
        << "); defaulting to grayscale" << endl;
    }
  }
  
//  if (tileMultiplier == -1) {
//    tileMultiplier = 1;
//    if (ncer.cebkBlock.tileIndexOffsetFlags & 0x01) tileMultiplier *= 2;
//    if (ncer.cebkBlock.tileIndexOffsetFlags & 0x02) tileMultiplier *= 2;
//  }
  
  nscr.saveToEditable(prefix, tiles.tiles_, pal, rotscale);
  
  return 0;
}

int patchImage(char* prefix, char* tile, char* outfile) {
  if (prefix == NULL) {
    cerr << "Error: missing input prefix" << endl;
    return 1;
  }
  else if (tile == NULL) {
    cerr << "Error: missing input NCGR file" << endl;
    return 1;
  }
  else if (outfile == NULL) {
    cerr << "Error: missing output file name" << endl;
    return 1;
  }

  TIfstream ifs(tile, ios_base::binary);
  NcgrFile ncgr;
  ncgr.read(ifs);
  ifs.close();
  NcgrTileSet tiles;
  tiles.fromNcgrFile(ncgr);

//  cout << outfile << endl;
  CebkBankEntry::patchTileSet(prefix, tiles.tiles_);
  tiles.toNcgrFile(ncgr);
  TBufStream testout(0x800000);
  ncgr.write(testout);
  testout.save(outfile);
  
  return 0;
}

int patchScreen(char* prefix, char* tile, char* outfile) {
  if (prefix == NULL) {
    cerr << "Error: missing input prefix" << endl;
    return 1;
  }
  else if (tile == NULL) {
    cerr << "Error: missing input NCGR file" << endl;
    return 1;
  }
  else if (outfile == NULL) {
    cerr << "Error: missing output file name" << endl;
    return 1;
  }

  TIfstream ifs(tile, ios_base::binary);
  NcgrFile ncgr;
  ncgr.read(ifs);
  ifs.close();
  NcgrTileSet tiles;
  tiles.fromNcgrFile(ncgr);

  NscrFile nscr;
  nscr.patchTileSetFromEditable(prefix, tiles.tiles_);
  
  tiles.toNcgrFile(ncgr);
  TBufStream testout(0x800000);
  ncgr.write(testout);
  testout.save(outfile);
  
  return 0;
}

int extractTileSet(char* prefix, char* palette, char* tile,
                  int palettenum) {
  if (prefix == NULL) {
    cerr << "Error: missing output prefix" << endl;
    return 1;
  }
  else if (tile == NULL) {
    cerr << "Error: missing input NCGR file" << endl;
    return 1;
  }
  
  TIfstream ifs(tile, ios_base::binary);
  NcgrFile ncgr;
  ncgr.read(ifs);
  NcgrTileSet tiles;
  tiles.fromNcgrFile(ncgr);
  
  NitroPalette pal = (tiles.tiles_.bpp() == 4)
    ? grayscale16 : grayscale256;
  if (palette != NULL) {
    TIfstream ifs3(palette, ios_base::binary);
    NclrFile nclr;
    nclr.read(ifs3);
    if (palettenum < nclr.plttBlock.palettes.size()) {
      pal = nclr.plttBlock.palettes[palettenum];
    }
    else {
      cout << "Out-of-range palette index " << palettenum
        << " (max " << (nclr.plttBlock.palettes.size() - 1)
        << "); defaulting to grayscale" << endl;
    }
  }
  
  tiles.tiles_.saveToEditable((string(prefix) + "tiles").c_str(),
                       pal);
  
  return 0;
}

int patchTileSet(char* prefix, char* tile, char* outfile) {
  if (prefix == NULL) {
    cerr << "Error: missing input prefix" << endl;
    return 1;
  }
  else if (tile == NULL) {
    cerr << "Error: missing input NCGR file" << endl;
    return 1;
  }
  else if (outfile == NULL) {
    cerr << "Error: missing output file name" << endl;
    return 1;
  }

  TIfstream ifs(tile, ios_base::binary);
  NcgrFile ncgr;
  ncgr.read(ifs);
  ifs.close();
  NcgrTileSet tiles;
  tiles.fromNcgrFile(ncgr);

  tiles.tiles_.loadFromEditable((string(prefix) + "tiles").c_str());
  tiles.toNcgrFile(ncgr);
  TBufStream testout(0x100000);
  ncgr.write(testout);
  testout.save(outfile);
  
  return 0;
}

int main(int argc, char* argv[]) {
/*  TIfstream tst(argv[1], ios_base::binary);
  tst.seek(0x76);
  TGraphic g(64, 64);
  for (int j = 63; j >= 0; j--) {
    for (int i = 0; i < 64; i++) {
      TByte val = tst.get();
      int value = (val << 4) | val;
      TColor color(value, value, value, TColor::fullAlphaOpacity);
      g.setPixel(i, j, color);
    }
  }
  TPngConversion::graphicToRGBAPng("test.png", g);
  return 0; */

  if (argc < 3) {
    cout << "Standard DS image format conversion tool" << endl;
    cout << "Usage: " << argv[0] << " <command> <prefix> [arguments]" << endl;
    cout << endl;
    cout << "Commands: " << endl;
    cout << "  e      Extract images from NCER\n";
    cout << "  es     Extract images from NSCR\n";
    cout << "  p      Patch NCGR using an extracted image\n";
    cout << "  ps     Patch NCGR using an extracted screen\n";
    cout << "  et     Extract raw contents of NCGR\n";
    cout << "  pt     Patch NCGR from raw contents\n";
    cout << endl;
    cout << "Options: " << endl;
    cout << "  -n     Specifies name of input/output file, if any\n";
    cout << "  -p     Specifies NCLR palette file (grayscale if omitted)\n";
    cout << "  -t     Specifies NCGR tile data file\n";
    cout << "  -c     Specifies NCER cell data file\n";
    cout << "  -s     Specifies NSCR screen data file\n";
    cout << endl;
    cout << "  -pn    Specifies index of sub-palette\n";
    cout << "  -tm    Forces tile multiplier to a specific value\n";
    cout << "  -rs    Sets rotation/scaling flag (0 or 1)\n";
    cout << "  -sp    For NCER rendering, sets OAM split point\n";
    
    return 0;
  }

  // Initialize grayscale palettes
  
  for (int i = 0; i < 16; i++) {
    int value = (i << 4) | i;
    grayscale16.setColor(i, TColor(value, value, value,
      (i == 0) ? TColor::fullAlphaTransparency : TColor::fullAlphaOpacity));
//    grayscale16.setColor(i, TColor(value, value, value,
//      TColor::fullAlphaOpacity));
  }
  
  for (int i = 0; i < 256; i++) {
    int value = i;
    grayscale256.setColor(i, TColor(value, value, value,
      (i == 0) ? TColor::fullAlphaTransparency : TColor::fullAlphaOpacity));
//    grayscale256.setColor(i, TColor(value, value, value,
//      TColor::fullAlphaOpacity));
  }
  
  char* command = argv[1];
  char* prefix = argv[2];
  
  char* iofile = getOpt(argc, argv, "-n");
  char* palette = getOpt(argc, argv, "-p");
  char* tile = getOpt(argc, argv, "-t");
  char* cell = getOpt(argc, argv, "-c");
  char* screen = getOpt(argc, argv, "-s");
  char* pn = getOpt(argc, argv, "-pn");
  char* tm = getOpt(argc, argv, "-tm");
  char* rs = getOpt(argc, argv, "-rs");
  char* sp = getOpt(argc, argv, "-sp");
  
  int palettenum = 0;
  if (pn != NULL) palettenum = TStringConversion::stringToInt(string(pn));
  
  int tilemult = -1;
  if (tm != NULL) tilemult = TStringConversion::stringToInt(string(tm));
  
  int splitpoint = -1;
  if (sp != NULL) splitpoint = TStringConversion::stringToInt(string(sp));
  
  bool rotscale = false;
  if (rs != NULL) rotscale
    = TStringConversion::stringToInt(string(rs)) != 0;
  
  if (strcmp(command, "e") == 0) {
    return extractImages(prefix, palette, tile, cell, palettenum, tilemult,
                         splitpoint);
  }
  else if (strcmp(command, "es") == 0) {
    return extractScreen(
      prefix, palette, tile, screen, palettenum, rotscale);
  }
  else if (strcmp(command, "p") == 0) {
    return patchImage(prefix, tile, iofile);
  }
  else if (strcmp(command, "ps") == 0) {
    return patchScreen(
      prefix, tile, iofile);
  }
  else if (strcmp(command, "et") == 0) {
    return extractTileSet(prefix, palette, tile, palettenum);
  }
  else if (strcmp(command, "pt") == 0) {
    return patchTileSet(prefix, tile, iofile);
  }
  else {
    cout << "Unknown command: " << command << endl;
    return 0;
  }
  
//  TIfstream ifs(argv[1], ios_base::binary);
//  NcgrFile ncgr;
//  ncgr.read(ifs);
  
//  NcgrTileSet tiles;
//  tiles.fromNcgrFile(ncgr);
  
//  cout << ncgr.hasCposBlock << endl;
  
  TIfstream ifs("opbout_test/0-0.bin", ios_base::binary);
  NcgrFile ncgr;
  ncgr.read(ifs);
  
//  TBufStream ofs(0x100000);
//  ncgr.write(ofs);
//  ofs.save("0-0-test.ncgr");
  
  NcgrTileSet tiles;
  tiles.fromNcgrFile(ncgr);

  TIfstream ifs2("opbout_test/0-2.bin", ios_base::binary);
  NcerFile ncer;
  ncer.read(ifs2);
  
  TGraphic test;
  for (int i = 0; i < ncer.cebkBlock.banks.size(); i++) {
/*    ncer.cebkBlock.banks[i].saveAsEditable((string("opbout/newtest-")
      + TStringConversion::intToString(i)).c_str(),
          tiles.tiles_, grayscale256, 2); */
      
/*    ncer.cebkBlock.banks[i].composeToGraphic(test,
          tiles.tiles_, grayscale256, 2);
    TPngConversion::graphicToRGBAPng("opbout/test-"
      + TStringConversion::intToString(i)
      + ".png", test); */
      
/*    for (int j = 0; j < ncer.cebkBlock.banks[i].oams.size(); j++) {
      ncer.cebkBlock.banks[i].oams[j].composeToGraphic(test,
        tiles.tiles_, grayscale256, 2);
      TPngConversion::graphicToRGBAPng("opbout/test-"
        + TStringConversion::intToString(i)
        + "-"
        + TStringConversion::intToString(j)
        + ".png", test);
    } */
  }
//  ncer.cebkBlock.banks[0].oams[0].composeToGraphic(test,
//    tiles.tiles_, grayscale256, 4);
//  TPngConversion::graphicToRGBAPng("test.png", test);
  
//  TBufStream ofs(0x100000);
//  ncer.write(ofs);
//  ofs.save("0-2-test.ncer");

  CebkBankEntry::patchTileSet("opbout/newtest-0", tiles.tiles_);
  tiles.toNcgrFile(ncgr);
  TBufStream testout(0x100000);
  ncgr.write(testout);
  testout.save("ncgr-patch-test.ncgr");
  
  return 0;
} 
