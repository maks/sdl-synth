#ifndef OSC_H
#define OSC_H

const int LUT_SIZE = 256;
const int SAMPLE_RATE = 44100;

class SineOsc {
public:
  SineOsc(){};
  int generateSample();

  void setFrequency(float freq) {
    // get phase increment for note depending on sample rate and LUT length
    phase_increment = (freq / SAMPLE_RATE) * LUT_SIZE;
  }

private:
  float phase_index = 0;
  float phase_increment = 1;
};

#endif