#include "tinysynth2.h"
#include "fixed.h"
#include "math.h"
#include "osc.h"
#include "utils.h"
#include <cstdint>
#include <stdio.h>
#include <string.h>

void TinySynth::generateWaves(uint8_t *byte_stream, int len) {
  if (adsr_state[0] == 6) {
    return;
  }

  int16_t *s_byte_stream;

  // update_envelopes();

  // get correct phase increment for note depending on sample rate and LUT
  // length.
  float baseNoteFreq = (noteToFreq(_note) / SAMPLE_RATE) * LUT_SIZE;

  // Harmonics of primary freq
  float phase_increment[HARMONICS] = {
      baseNoteFreq,     2 * baseNoteFreq, 3 * baseNoteFreq,
      4 * baseNoteFreq, 5 * baseNoteFreq, 6 * baseNoteFreq,
  };

  /* cast buffer as 16bit signed int */
  s_byte_stream = (int16_t *)byte_stream;

  // generate samples
  for (int i = 0; i < len; i++) {
    int fullResult = 0;
    // for (int h = 0; h < HARMONICS; h++) {
    // fullResult += osc.generateSample();
    // }
    // write sum of all harmonics into audio buffer
    fullResult = osc.generateSample();
    s_byte_stream[i] = fullResult << 8;
  }
}

void TinySynth::envelope_gate(bool on) {
  char state = on ? 0 : 6;
  for (int h = 0; h < HARMONICS; h++) {
    adsr_state[h] = state;
  }
}

void TinySynth::set_note(char note) { _note = note; }

char TinySynth::get_note() { return _note; }
