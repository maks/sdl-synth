#ifndef OSC_H
#define OSC_H

const int LUT_SIZE = 256;

class Osc {
public:
  Osc() {}
  int generateSample();

private:
  float phase_index = 0;
  float phase_increment = 1;
};

#endif