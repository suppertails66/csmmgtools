#include "grayman/GrayManLineWrapper.h"
#include "grayman/GrayManConsts.h"
#include "util/TParse.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <cctype>
#include <iostream>

using namespace BlackT;

namespace Nftred {

//const static int code_space   = 0x20;
//
//const static int code_wait      = 0x81A5;
//const static int code_br        = 0xA;
//const static int code_end       = 0x00;

GrayManLineWrapper::GrayManLineWrapper(BlackT::TStream& src__,
                ResultCollection& dst__,
                const BlackT::TThingyTable& thingy__,
                CharSizeTable sizeTable__,
                int xSize__,
                int ySize__)
  : TLineWrapper(src__, dst__, thingy__, xSize__, ySize__),
    sizeTable(sizeTable__),
    currentFontPage(0) {
  
}

int GrayManLineWrapper::widthOfKey(int key) {
  if ((key == GrayManConsts::code_wait))
    return 0;
  else if ((key == GrayManConsts::code_br))
    return 0;
  else if ((key == GrayManConsts::code_end))
    return 0;
  else if ((key == 0x81A3))
    return 0;
  
  if (key < 0x100) return sizeTable[(currentFontPage << 8) | key];
  else {
/*    std::string errstring = std::string("Line ")
                              + TStringConversion::intToString(lineNum)
                              + ": illegal key '"
                              + TStringConversion::intToString(key,
                                  TStringConversion::baseHex)
                              + "'";
    std::cerr << errstring << std::endl;
    return 0; */
    throw TGenericException(T_SRCANDLINE,
                            "GrayManLineWrapper::widthOfKey()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ": illegal key '"
                              + TStringConversion::intToString(key,
                                  TStringConversion::baseHex)
                              + "'");
  }
//  return 8;
//  return sizeTable[key];
}

bool GrayManLineWrapper::isWordDivider(int key) {
  if (
      (key == GrayManConsts::code_br)
      || (key == GrayManConsts::code_space)
     ) return true;
  
  return false;
}

bool GrayManLineWrapper::isLinebreak(int key) {
  if (
      (key == GrayManConsts::code_br)
      ) return true;
  
  return false;
}

bool GrayManLineWrapper::isBoxClear(int key) {
  if ((key == GrayManConsts::code_end)
      ) return true;
  
  return false;
}

void GrayManLineWrapper::onBoxFull() {
/*  std::string content;
  if (lineHasContent) {
    // wait
    content += thingy.getEntry(code_wait);
    content += linebreakString();
    currentScriptBuffer.write(content.c_str(), content.size());
  }
  // linebreak
  stripCurrentPreDividers();
  
  currentScriptBuffer.put('\n');
  xPos = 0;
//  yPos = -1;
  yPos = 0; */

  std::cerr << "WARNING: line " << lineNum << ":" << std::endl;
  std::cerr << "  overflow at: " << std::endl;
  std::cerr << streamAsString(currentScriptBuffer)
    << std::endl
    << streamAsString(currentWordBuffer) << std::endl;
  
  // HACK: suppress repeated errors (sort of)
  xPos = 0;
//  yPos = -1;
  yPos = 0;
}

//int GrayManLineWrapper::linebreakKey() {
//  return code_br;
//}

std::string GrayManLineWrapper::linebreakString() const {
  return thingy.getEntry(GrayManConsts::code_br);
}

TLineWrapper::Symbol GrayManLineWrapper
  ::fetchNextSymbol(BlackT::TStream& ifs) {
  // we want to copy command sequences (<WX...>, #XX) directly to the
  // output with no processing when they are encountered
  TLineWrapper::Symbol result;
  result.raw = true;
  if ((ifs.peek() == '#')
      && (ifs.remaining() >= 2)) {
    result.litstr += ifs.get();
    result.litstr += ifs.get();
    result.litstr += ifs.get();
    return result;
  }
  else if (ifs.peek() == '<') {
    int pos = ifs.tell();
    
    std::string resultString;
    resultString += ifs.get();
    
    bool matched = false;
    while (!ifs.eof()) {
      char next = ifs.get();
      resultString += next;
      if (next == '>') {
        result.litstr = resultString;
        matched = true;
        
        // switch font page if "<cX>"
        if ((resultString.size() == 4)
            && ((resultString[1] == 'c')
                || (resultString[1] == 'C'))
            && isdigit(resultString[2])) {
          currentFontPage = TStringConversion::stringToInt(
            std::string() + resultString[2]);
        }
        
        break;
      }
    }
    
    // failure
    if (!matched) {
      ifs.clear();
      ifs.seek(pos);
    }
    else return result;
  }
  
  // HACK: the full SJIS table lookup is slow, and causes unacceptable
  // slowdown for large input sizes (such as this game's script.)
  // we know all control sequences start with '[' and everything else is just
  // plain SJIS, so we can skip the full lookup in most cases.
/*  result.raw = false;
  if ((ifs.peek() != '[')
      && ((unsigned int)(unsigned char)ifs.peek() < 0x80)) {
    result.id = ifs.readu8();
    return result;
  }
  else if ((unsigned int)(unsigned char)ifs.peek() >= 0x80) {
    result.id = ifs.readu16be();
    return result;
  } */
  
  return TLineWrapper::fetchNextSymbol(ifs);
}

/*void GrayManLineWrapper::outputLinebreak(std::string str) {
  ++trueY;
  TLineWrapper::outputLinebreak(str);
}

void GrayManLineWrapper::onSymbolAdded(BlackT::TStream& ifs, int key) {
  if (waitPending) {
    if ((key != code_end)
        && (key != code_clear)) {
//      std::cerr << "here: " << std::hex << key << std::endl;
      outputLinebreak(linebreakString());
    }
    
    waitPending = false;
  }
  
  // if box previously filled, and will now be reset, mark as unfilled
//  if (boxFilled
//      && ((key == code_end) || (key == code_clear))) {
//    boxFilled = false;
//  }
}

void GrayManLineWrapper
    ::handleManualLinebreak(TLineWrapper::Symbol result, int key) {
//  if ((key != code_br) || (breakMode == breakMode_single)) {
//  }
//  else {
//    outputLinebreak(linebreakString());
//  }
  if (key == code_br) {
    outputLinebreak(linebreakString());
  }
  else {
    TLineWrapper::handleManualLinebreak(result, key);
  }
}

void GrayManLineWrapper::afterLinebreak(
    LinebreakSource clearSrc, int key) {
  
}

void GrayManLineWrapper::beforeBoxClear(
    BoxClearSource clearSrc, int key) {
  if ((clearSrc == boxClearManual)
      && ((key == code_wait) || (key == code_wait2))) {
    xBeforeWait = xPos;
    yBeforeWait = yPos;
  }
  else if ((clearSrc == boxClearManual)
      && ((key == code_clear) || (key == code_end))) {
    trueY = 0;
  }
}

void GrayManLineWrapper::afterBoxClear(
  BoxClearSource clearSrc, int key) {
  if (((clearSrc == boxClearManual)
      && ((key == code_wait) || (key == code_wait2)))) {
//    xPos = xBeforeWait;
    yPos = -1;

    // y is preserved if box was "cleared" with wait.
    // why was box scrolling implemented like this, it breaks all my
    // line wrapping assumptions
//    yPos = yBeforeWait;

    // inject linebreak after manual wait if content remains
    waitPending = true;
  }
} */

bool GrayManLineWrapper::processUserDirective(BlackT::TStream& ifs) {
  TParse::skipSpace(ifs);
  
  std::string name = TParse::matchName(ifs);
  TParse::matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
/*  if (name.compare("SETBREAKMODE") == 0) {
    std::string type = TParse::matchName(ifs);
    
    if (type.compare("SINGLE") == 0) {
      breakMode = breakMode_single;
    }
    else if (type.compare("DOUBLE") == 0) {
      breakMode = breakMode_double;
    }
    else {
      throw TGenericException(T_SRCANDLINE,
                              "GrayManLineWrapper::processUserDirective()",
                              "Line "
                                + TStringConversion::intToString(lineNum)
                                + ": unknown break mode '"
                                + type
                                + "'");
    }
    
    return true;
  }
  else if (name.compare("SETBOXMODE") == 0) {
    std::string type = TParse::matchName(ifs);
    
    if (type.compare("MODE1") == 0) {
      boxMode = boxMode_1;
    }
    else if (type.compare("MODE2") == 0) {
      boxMode = boxMode_2;
    }
    else {
      throw TGenericException(T_SRCANDLINE,
                              "GrayManLineWrapper::processUserDirective()",
                              "Line "
                                + TStringConversion::intToString(lineNum)
                                + ": unknown box mode '"
                                + type
                                + "'");
    }
    
    return true;
  } */
//  else if (name.compare("ENDMSG") == 0) {
//    processEndMsg(ifs);
//    return true;
//  }
  
  return false;
}

}
