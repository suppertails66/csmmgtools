#ifndef NITROPALETTE_H
#define NITROPALETTE_H


#include "util/TColor.h"
#include "util/TStream.h"
#include "util/TGraphic.h"
#include <map>
//#include <unordered_map>

namespace Nftred {


//class fuckshit {
//public:
//  int operator()(const BlackT::TColor& col) const;
//};

class NitroPalette {
public:
  NitroPalette();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  void saveToEditable(const char* prefix) const;
  void loadFromEditable(const char* prefix);
  
  void generatePreview(BlackT::TGraphic& dst) const;
  
  void setColor(int index, BlackT::TColor color);
  
  bool hasColor(int index) const;
  
  int numColors() const;
  
  BlackT::TColor color(int index) const;
  int indexOfColor(BlackT::TColor color) const;
  
protected:
  const static int previewRectW = 8;
  const static int previewRectH = 8;
  const static int previewRectsPerRow = 16;
  
  static int colorhash(BlackT::TColor);

  std::map<int, BlackT::TColor> indexToColor_;
  std::map<BlackT::TColor, int> colorToIndex_;
};


}


#endif
