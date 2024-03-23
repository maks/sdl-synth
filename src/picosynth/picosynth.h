#ifndef PICO_SYNTH_H
#define PICO_SYNTH_H

#include <cstdint>

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 256;
const int UPDATE_RATE = SAMPLE_RATE / BUFFER_SIZE;

const int HARMONICS = 6;
const int LUT_SIZE = 100;

float noteToFreq(char note);

class PicoSynth {
public:
  PicoSynth() {}

  void generateWaves(uint8_t *byte_stream, int len);

  void update_envelopes();

  void set_defaults();

  void envelope_gate(bool on);

  void set_note(char note);

  char get_note();

private:
  // TODO: convert env & wave into arrays of structs

  // The current envelope settings
  // env[h][0] is envelope type
  // env[h][1] is envelope attack
  // env[h][2] is envelope decay
  // env[h][3] is envelope sustain
  // env[h][4] is envelope release
  uint16_t env[HARMONICS][5];

  // The current per harmonic wave settings
  // wave[h][0] is oscillator amplitude
  // wave[h][1] is initialise the phase (offset into the sine LUT) of the wave
  uint16_t wave[HARMONICS][2];

  /*
   * The current envelope settings. This represents the volume of the harmonic,
   * and the state it is in.
   */
  int filt[HARMONICS] = {0, 0, 0, 0, 0, 0};
  char filt_state[HARMONICS] = {0, 0, 0, 0, 0, 0};
  float phase_int[HARMONICS] = {0, 0, 0, 0, 0, 0};

  char gate = 0;
  int _note = 60; /* integer representing midi notes */

  int generatePhaseSample(float phase_increment, float &phase_index, int vol);
};

#endif