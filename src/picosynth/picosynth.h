#ifndef PICO_SYNTH_H
#define PICO_SYNTH_H

#include <cstdint>
#include <math.h>

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 256;
// want envelope update rate of ~10ms aka ~100Hz
const int ENVELOPE_UPDATE_RATE = floor(((float)SAMPLE_RATE / BUFFER_SIZE) / 10);

const int HARMONICS = 6;
const int LUT_SIZE = 100;

float noteToFreq(char note);

struct picosynth_env {
  char type;
  int attack;
  int decay;
  int sustain;
  int release;
  int amplitude;
  int phase;
};

class PicoSynth {
public:
  PicoSynth() {}

  void setEnvelopeConfig(char index, picosynth_env config);

  picosynth_env getEnvelopeConfig(char index);

  void generateWaves(uint8_t *byte_stream, int len);

  void update_envelopes();

  void set_defaults();

  void envelope_gate(bool on);

  void set_note(char note);

  char get_note();

private:
  picosynth_env env[HARMONICS];
  // The current envelope settings. This represents the volume of the harmonic,
  // and the state it is in.
  int filt[HARMONICS] = {0, 0, 0, 0, 0, 0};
  // make sure we start in note-off state
  char filt_state[HARMONICS] = {6, 6, 6, 6, 6, 6};
  float phase_int[HARMONICS] = {0, 0, 0, 0, 0, 0};

  char gate = 0;
  char _note = 60; /* integer representing midi notes */
  char _env_update_count = 0;

  int generatePhaseSample(float phase_increment, float &phase_index, int vol);
};

#endif