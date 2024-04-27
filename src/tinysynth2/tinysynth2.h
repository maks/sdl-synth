#ifndef TINY_SYNTH_H
#define TINY_SYNTH_H

#include "adsr.h"
#include "osc.h"
#include <cstdint>
#include <math.h>

const int BUFFER_SIZE = 512;
const int CONTROL_RATE_DIVISOR = 10;
const int CONTROL_RATE = SAMPLE_RATE / CONTROL_RATE_DIVISOR;

const int HARMONICS = 6;

float noteToFreq(char note);

struct tinysynth_env {
  char type;
  char attack;
  char decay;
  char sustain;
  char release;
  char amplitude;
  char phase;
};

class TinySynth {
public:
  TinySynth() {
    adsr.setAttackRate(0);
    adsr.setSustainLevel(255);
    adsr.setReleaseRate(CONTROL_RATE * 1.5);
  };

  void setEnvelopeConfig(char index, tinysynth_env config);

  tinysynth_env getEnvelopeConfig(char index);

  void generateWaves(uint8_t *byte_stream, int len);

  void update_envelopes();

  void set_defaults();

  void envelope_gate(bool on);

  void set_note(char note);

  char get_note();

private:
  tinysynth_env env[HARMONICS];
  // The current envelope settings. This represents the volume of the harmonic,
  // and the state it is in.
  int filt[HARMONICS] = {0, 0, 0, 0, 0, 0};
  // make sure we start in note-off state
  char adsr_state[HARMONICS] = {6, 6, 6, 6, 6, 6};
  float phase_int[HARMONICS] = {0, 0, 0, 0, 0, 0};

  char _note = 60; /* integer representing midi notes */

  SineOsc osc = SineOsc();
  ADSR adsr = ADSR();
};

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

#endif