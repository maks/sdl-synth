#include "osc.h"
#include "tinysynth2.h"
#include <stdio.h>

int Osc::generateSample() {
  phase_index += phase_increment;

  if (phase_index >= LUT_SIZE) {
    int diff = phase_index - LUT_SIZE;
    phase_index = diff;
  }

  int result = (int)sine[(int)phase_index];
  return result;
}