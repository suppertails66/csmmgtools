#include "util/TBufStream.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include <cstring>

namespace BlackT {


TBufStream::TBufStream(int sz)
  : pos_(0) {
  data_.resize(sz);
  std::memset(data_.data(), sz, 0);
}

TBufStream::~TBufStream() { }
  
void TBufStream::open(const char* filename) {
  TIfstream ifs(filename, std::ios_base::binary);
  data_.resize(ifs.size());
  ifs.read(data_.data(), data_.size());
}

void TBufStream::save(const char* filename) {
  TOfstream ofs(filename, std::ios_base::binary);
  ofs.write(data_.data(), pos_);
}

char TBufStream::get() {
  return data_[pos_++];
}

void TBufStream::unget() {
  --pos_;
}

void TBufStream::put(char c) {
  data_[pos_++] = c;
}

void TBufStream::read(char* dst, int size) {
  std::memcpy(dst, data_.data() + pos_, size);
  pos_ += size;
}

void TBufStream::write(const char* src, int size) {
  std::memcpy(data_.data() + pos_, src, size);
  pos_ += size;
}

bool TBufStream::good() const {
  return (bad() || fail() || eof());
}

bool TBufStream::bad() const {
  return false;
}

bool TBufStream::fail() const {
  return eof();
}

bool TBufStream::eof() const {
  return (pos_ >= data_.size());
}

int TBufStream::tell() {
  return pos_;
}

void TBufStream::seek(int pos) {
  pos_ = pos;
}

int TBufStream::size() {
  return data_.size();
}

void TBufStream::alignToBoundary(int byteBoundary) {
  int amt = byteBoundary - (pos_ % byteBoundary);
  if (amt != byteBoundary) {
    for (int i = 0; i < amt; i++) {
      put(0);
    }
  }
}
  
void TBufStream::writeFrom(TStream& ifs, int sz) {
  ifs.read((char*)(data_.data() + pos_), sz);
  pos_ += sz;
}

TArray<char>& TBufStream::data() {
  return data_;
}

const TArray<char>& TBufStream::data() const {
  return data_;
}


}
