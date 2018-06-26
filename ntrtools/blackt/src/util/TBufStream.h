#ifndef TBUFSTREAM_H
#define TBUFSTREAM_H


#include "util/TStream.h"
#include "util/TArray.h"

namespace BlackT {


class TBufStream : public TStream {
public:
  TBufStream(int sz);
  virtual ~TBufStream();
  
  virtual void open(const char* filename);
  virtual void save(const char* filename);
  
  virtual char get();
  virtual void unget();
  virtual void put(char c);
  virtual void read(char* dst, int size);
  virtual void write(const char* src, int size);
  virtual bool good() const;
  virtual bool bad() const;
  virtual bool fail() const;
  virtual bool eof() const;
  virtual int tell();
  virtual void seek(int pos);
  virtual int size();
  
  virtual void alignToBoundary(int byteBoundary);
  
  virtual void writeFrom(TStream& ifs, int sz);
  
  TArray<char>& data();
  const TArray<char>& data() const;
  
protected:
  TArray<char> data_;
  int pos_;
};


}


#endif
