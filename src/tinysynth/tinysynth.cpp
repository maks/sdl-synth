#include "tinysynth.h"
#include "fixed.h"
#include "math.h"
#include <cstdint>
#include <stdio.h>
#include <string.h>

const char sine[LUT_SIZE] = {
    0,    3,    6,    9,    12,   15,   18,   21,   24,   27,   30,   33,
    36,   39,   42,   45,   48,   51,   54,   57,   59,   62,   65,   67,
    70,   73,   75,   78,   80,   82,   85,   87,   89,   91,   94,   96,
    98,   100,  102,  103,  105,  107,  108,  110,  112,  113,  114,  116,
    117,  118,  119,  120,  121,  122,  123,  123,  124,  125,  125,  126,
    126,  126,  126,  126,  127,  126,  126,  126,  126,  126,  125,  125,
    124,  123,  123,  122,  121,  120,  119,  118,  117,  116,  114,  113,
    112,  110,  108,  107,  105,  103,  102,  100,  98,   96,   94,   91,
    89,   87,   85,   82,   80,   78,   75,   73,   70,   67,   65,   62,
    59,   57,   54,   51,   48,   45,   42,   39,   36,   33,   30,   27,
    24,   21,   18,   15,   12,   9,    6,    3,    0,    -3,   -6,   -9,
    -12,  -15,  -18,  -21,  -24,  -27,  -30,  -33,  -36,  -39,  -42,  -45,
    -48,  -51,  -54,  -57,  -59,  -62,  -65,  -67,  -70,  -73,  -75,  -78,
    -80,  -82,  -85,  -87,  -89,  -91,  -94,  -96,  -98,  -100, -102, -103,
    -105, -107, -108, -110, -112, -113, -114, -116, -117, -118, -119, -120,
    -121, -122, -123, -123, -124, -125, -125, -126, -126, -126, -126, -126,
    -127, -126, -126, -126, -126, -126, -125, -125, -124, -123, -123, -122,
    -121, -120, -119, -118, -117, -116, -114, -113, -112, -110, -108, -107,
    -105, -103, -102, -100, -98,  -96,  -94,  -91,  -89,  -87,  -85,  -82,
    -80,  -78,  -75,  -73,  -70,  -67,  -65,  -62,  -59,  -57,  -54,  -51,
    -48,  -45,  -42,  -39,  -36,  -33,  -30,  -27,  -24,  -21,  -18,  -15,
    -12,  -9,   -6,   -3,
};

int TinySynth::generatePhaseSample(float phase_increment, float &phase_index,
                                   int vol) {
  phase_index = phase_index;
  phase_index += phase_increment;

  if (phase_index >= LUT_SIZE) {
    int diff = phase_index - LUT_SIZE;
    phase_index = diff;
  }

  int result = (int)sine[(int)phase_index] * vol >> 8;
  return result;
}

// Calculate frequency from MIDI note value
// ref: https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd
float noteToFreq(char note) {
  float a = 440; // frequency of A4 (440Hz)
  return (a / 32) * pow(2, ((note - 9) / 12.0));
}

void TinySynth::setEnvelopeConfig(char index, tinysynth_env config) {
  env[index] = config;
}

tinysynth_env TinySynth::getEnvelopeConfig(char index) { return env[index]; }

void TinySynth::generateWaves(uint8_t *byte_stream, int len) {
  int16_t *s_byte_stream;

  update_envelopes();

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
    for (int h = 0; h < HARMONICS; h++) {
      fullResult +=
          generatePhaseSample(phase_increment[h], phase_int[h], filt[h]);
    }
    // write sum of all harmonics into audio buffer
    s_byte_stream[i] = fullResult;
  }
}

void TinySynth::envelope_gate(bool on) {
  char state = on ? 0 : 4;
  for (int h = 0; h < HARMONICS; h++) {
    filt_state[h] = state;
  }
}

void TinySynth::set_note(char note) { _note = note; }

char TinySynth::get_note() { return _note; }

void TinySynth::set_defaults() {

  for (int h = 0; h < HARMONICS; h++) {
    env[h].attack = 30;
    env[h].sustain = 30;
    env[h].release = 20;
  }
  // sawtooth
  // int saw_vol = 240;
  // env[0].amplitude = saw_vol;
  // env[1].amplitude = saw_vol / 2;
  // env[2].amplitude = saw_vol / 3;
  // env[3].amplitude = saw_vol / 4;
  // env[4].amplitude = saw_vol / 5;
  // env[5].amplitude = saw_vol / 6;

  // Square
  // int square_vol = 4000;
  // env[0].amplitude = square_vol;
  // env[2].amplitude = square_vol / 3;
  // env[4].amplitude = square_vol / 5;

  // Sine
  // env[0].amplitude = 150;
  env[0].amplitude = 100;
}

/*
 * Needs to be called regularly (eg every buffer fill), this updates the
 * individual harmonic volumes in line with their Envelope. There are 4 envelope
 * types. These are ADSR, delayed ADSR, AD repeat and delayed AD repeat. The
 * delayed envelope waits to go into the decay phase until all active
 * oscillators have finished their attack phase.
 *
 * State 0 => attack
 * State 1 => delay
 * State 2 => decay
 * State 3 => sustain
 * State 4 => release
 * State 5 => note off
 *
 * A standard adsr will go, states: 0, 2, 3, 4, 5
 */
void TinySynth::update_envelopes() {
  uint8_t finished = 0;
  uint8_t playing = HARMONICS;
  int attack, decay, level, sustain, rel;
  uint8_t etype;
  uint8_t tremolo = 1;

  /*
   * Work out which notes are still playing, and which have
   * finished their attack phase. Repeating envelopes don't
   * count towards a playing note as they repeat indefinitely.
   */
  for (char i = 0; i < HARMONICS; i++) {
    if (env[i].type < 2 && env[i].type)
      tremolo = 0;
    if (filt_state[i] > 0 || env[i].type > 1) {
      finished++;
      if (filt_state[i] == 5 || env[i].type > 1)
        playing--;
    }
  }

  /*
   * Loop through the oscillators, and set their filt value. The
   * filt value represents the current volume of the oscillator.
   */
  for (uint8_t i = 0; i < HARMONICS; i++) {
    level = env[i].amplitude << 8;
    if (level == 0) {
      filt_state[i] = 6;
      continue;
    }
    etype = (uint8_t)(env[i].type);
    attack = level / env[i].attack;
    decay = env[i].decay;
    sustain = level >> 8 * env[i].sustain;
    rel = level / env[i].release;
    switch (filt_state[i]) {
    case 0: // Attack
      filt[i] += attack;
      if (attack == 0 || filt[i] >= level) {
        filt[i] = level;
        filt_state[i] = (etype & 1) ? 1 : 2;
      }
      break;
    case 1: // Delay
      if (finished == 6)
        filt_state[i] = 2;
      else
        break;
    case 2: // Decay
      filt[i] -= decay;
      if (decay == 0 || filt[i] <= sustain) {
        filt[i] = sustain;
        if (etype > 1 && etype < 4) {
          if (playing || tremolo)
            filt_state[i] = 0;
          else
            filt_state[i] = 6;
        } else {
          filt_state[i] = 3;
        }
      }
      break;
    case 3: // Sustain stay here until told to release
      if (filt[i] == 0)
        filt_state[i] = 6;
      break;
    case 4: // Release
      if (etype == 4) {
        filt[i] += rel;
        if (rel == 0 || filt[i] >= level) {
          filt[i] = level;
          filt_state[i] = 5;
        }
      } else {
        // printf("%dREL[%d]\n", i, filt[i]);
        filt[i] -= rel;
        if (rel == 0 || filt[i] <= 0) {
          filt[i] = 0;
          filt_state[i] = 6;
        }
      }
      break;
    case 5: // Reverse Release
      filt[i] -= decay;
      if (rel == 0 || filt[i] <= 0) {
        filt[i] = 0;
        filt_state[i] = 6;
      }
      break;
    default:       // Note off
      filt[i] = 0; // take care of repeating oscillators
      break;
    }
  }
}