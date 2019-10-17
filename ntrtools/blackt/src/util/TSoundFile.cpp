#include "util/TSoundFile.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include <cstring>
#include <iostream>

namespace BlackT {


TSoundFile::TSoundFile()
  : channels_(1),
    rate_(-1) {
  
}

TSoundFile::~TSoundFile() { }
  
void TSoundFile::exportWav(TStream& ofs) {
  int bitsPerSample = 16;
  int bytesPerSample = bitsPerSample / 8;
  int sampleRange = ((1 << bitsPerSample) / 2) - 1;
  int numSamples = data_.size() / channels_;
  
  // RIFF
  ofs.write("RIFF", 4);
  ofs.writeu32le(36 + (data_.size() * bytesPerSample));
  ofs.write("WAVE", 4);
  
  // fmt
  ofs.write("fmt ", 4);
  ofs.writeu32le(16);
  ofs.writeu16le(1);    // format = PCM
  ofs.writeu16le(channels_);
  ofs.writeu32le(rate_);
  ofs.writeu32le(rate_ * channels_ * bytesPerSample);
  ofs.writeu16le(channels_ * bytesPerSample);
  ofs.writeu16le(bitsPerSample);
  
  // data
  ofs.write("data", 4);
  ofs.writeu32le((data_.size() * bytesPerSample));
  int pos = 0;
  for (int i = 0; i < numSamples; i++) {
    
    for (int j = 0; j < channels_; j++) {
      int rendered = data_[pos++] * (double)sampleRange;
      ofs.writeInt(rendered, bytesPerSample,
        EndiannessTypes::little, SignednessTypes::sign);
    }
    
  }
  
}

int TSoundFile::channels() const {
  return rate_;
}

void TSoundFile::setChannels(int channels__) {
  channels_ = channels__;
}

int TSoundFile::rate() const {
  return rate_;
}

void TSoundFile::setRate(int rate__) {
  rate_ = rate__;
}

void TSoundFile::addSample(TSample sample) {
  data_.push_back(sample);
}

std::vector<double>& TSoundFile::data() {
  return data_;
}

const std::vector<double>& TSoundFile::data() const {
  return data_;
}


}
