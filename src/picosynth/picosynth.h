#ifndef PICO_SYNTH_H
#define PICO_SYNTH_H

#include <cstdint>

const int SAMPLE_RATE = 44100;

const int BUFFER_SIZE = 256;

void generateWaves(uint8_t *byte_stream);

void update_envelopes();

void set_defaults();

void envelope_gate(bool on);

void set_note(char note);

char get_note();

float noteToFreq(char note);

#endif