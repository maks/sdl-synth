#include "tinysynth2.h"
#include "adsr.h"
#include "fixed.h"
#include "math.h"
#include "osc.h"
#include "utils.h"
#include <cstdint>
#include <stdio.h>
#include <string.h>

void TinySynth::generateWaves(uint8_t *byte_stream, int len) {
  if (adsr.getState() == env_idle) {
    return;
  }

  int16_t *s_byte_stream;
  float baseNoteFreq = noteToFreq(_note);

  // Harmonics of primary freq
  float phase_increment[HARMONICS] = {
      baseNoteFreq,     2 * baseNoteFreq, 3 * baseNoteFreq,
      4 * baseNoteFreq, 5 * baseNoteFreq, 6 * baseNoteFreq,
  };

  osc.setFrequency(baseNoteFreq);

  /* cast buffer as 16bit signed int */
  s_byte_stream = (int16_t *)byte_stream;

  int env = adsr.process();

  // generate samples
  for (int i = 0; i < len; i++) {
    int fullResult = 0;
    // for (int h = 0; h < HARMONICS; h++) {
    // fullResult += osc.generateSample();
    // }
    // write sum of all harmonics into audio buffer
    // control rate of samplerate / 10
    if (i % CONTROL_RATE_DIVISOR == 0) {
      env = adsr.process();
    }
    fullResult = osc.generateSample();
    s_byte_stream[i] = fullResult * (env >> 8);
  }
}

void TinySynth::envelope_gate(bool on) {
  char state = on ? 0 : 6;
  for (int h = 0; h < HARMONICS; h++) {
    adsr_state[h] = state;

    adsr.gate(state);
  }
}

void TinySynth::set_note(char note) { _note = note; }

char TinySynth::get_note() { return _note; }
