#include "adsr.h"
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

int ADSR::process() {
  switch (state) {
  case env_idle:
    break;
  case env_attack:
    output += attackIncrement;
    if (output >= MAX_LEVEL || attackIncrement == 0) {
      output = MAX_LEVEL;
      state = env_decay;
    }
    break;
  case env_decay:
    output -= decayIncrement;
    if (output <= sustainLevel || sustainLevel == 0) {
      output = sustainLevel;
      state = env_sustain;
    }
    break;
  case env_sustain:
    // sustain here until the gate moves into release state
    break;
  case env_release:
    output -= releaseIncrement;
    if (output <= 0 || releaseIncrement == 0) {
      output = 0;
      state = env_idle;
    }
  }
  return output;
}

// attack duration in number of cycles at the control rate
// the control rate is the rate at which process() is called
void ADSR::setAttackRate(int duration) {
  if (duration == 0) {
    attackIncrement = 0;
    return;
  }
  attackIncrement = MAX_LEVEL / duration;
}

void ADSR::setDecayRate(int duration) { decayIncrement = MAX_LEVEL / duration; }

void ADSR::setReleaseRate(int duration) {
  releaseIncrement = MAX_LEVEL / duration;
}

void ADSR::setSustainLevel(int level) { sustainLevel = level; }

void ADSR::gate(bool gate) {
  if (gate)
    state = env_attack;
  else if (state != env_idle)
    state = env_release;
}

void ADSR::reset() {
  state = env_idle;
  output = 0.0;
}