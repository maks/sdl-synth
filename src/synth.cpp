#include <cstdio>
#include <cassert>

#include "SDL2/SDL_audio.h"
#include "fixed.h"
#include <SDL2/SDL.h>

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 256;

//=== PICO SYNTH

const int HARMONICS = 6;
const int LUT_SIZE = 100;

Sint16 sine[LUT_SIZE] = {
    0,      2057,   4106,   6139,   8148,   10125,  12062,  13951,  15785,
    17557,  19259,  20886,  22430,  23886,  25247,  26509,  27666,  28713,
    29648,  30465,  31163,  31737,  32186,  32508,  32702,  32767,  32702,
    32508,  32186,  31737,  31163,  30465,  29648,  28713,  27666,  26509,
    25247,  23886,  22430,  20886,  19259,  17557,  15785,  13951,  12062,
    10125,  8148,   6139,   4106,   2057,   0,      -2057,  -4106,  -6139,
    -8148,  -10125, -12062, -13951, -15785, -17557, -19259, -20886, -22430,
    -23886, -25247, -26509, -27666, -28713, -29648, -30465, -31163, -31737,
    -32186, -32508, -32702, -32767, -32702, -32508, -32186, -31737, -31163,
    -30465, -29648, -28713, -27666, -26509, -25247, -23886, -22430, -20886,
    -19259, -17557, -15785, -13951, -12062, -10125, -8148,  -6139,  -4106,
    -2057,
};

/*
 * The current envelope settings. This represents the volume of the harmonic,
 * and the state it is in.
 */
int filt[HARMONICS] = {0, 0, 0, 0, 0, 0};
u_char filt_state[HARMONICS] = {0, 0, 0, 0, 0, 0};
int phase_int[HARMONICS] = {0, 0, 0, 0, 0, 0};

int generatePhaseSample(int phase_increment, int &phase_int, int vol) {
  int result = 0;
  phase_int = phase_int;
  phase_int += phase_increment;

  if (phase_int >= LUT_SIZE) {
    int diff = phase_int - LUT_SIZE;
    phase_int = diff;
  }

  result = sine[phase_int];
  int result_fp = i2fp(result);
  auto res_fp = fp_mul(result_fp, vol);
  return fp2i(res_fp);
}

void generateWaves(Uint8 *byte_stream) {
  Sint16 *s_byte_stream;

  // auto phase_increment1 = ((float)440 / SAMPLE_RATE) * LUT_SIZE;
  // auto phase_increment2 = ((float)880 / SAMPLE_RATE) * LUT_SIZE;
  // auto phase_increment3 = ((float)1320 / SAMPLE_RATE) * LUT_SIZE;
  // printf("INC1:%f INC2:%f\n INC3:%f\n", phase_increment1, phase_increment2,
  //        phase_increment3);
  // exit(1);

  // Harmonics of primary freq, close to ordinals for a 100 sample sine LUT
  int phase_increment[HARMONICS] = {1, 2, 3, 4, 5, 6};

  // max 42038 across all harmonics
  int volume[HARMONICS] = {7000, 7000, 7000, 7000, 7000, 7000};

  /* cast buffer as 16bit signed int */
  s_byte_stream = (Sint16 *)byte_stream;

  // generate samples
  for (int i = 0; i < BUFFER_SIZE; i++) {
    int fullResult = 0;
    for (int h = 0; h < HARMONICS; h++) {
      fullResult +=
          generatePhaseSample(phase_increment[h], phase_int[h], volume[h]);
    }
    // write sum of all harmonics into audio buffer
    s_byte_stream[i] = fullResult;
  }
}
//=== PICO SYNTH

void oscillator_callback(void *userdata, Uint8 *byteStream, int len) {
  generateWaves(byteStream);
}

int main(int argc, char const *argv[])
{
  printf("sdl synth\n");

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
  {
    printf("Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  SDL_AudioSpec spec = {
      .freq = SAMPLE_RATE,
      .format = AUDIO_S16LSB,
      .channels = 1,
      .samples = BUFFER_SIZE,
      .callback = oscillator_callback,
  };

  if (SDL_OpenAudio(&spec, NULL) < 0)
  {
    printf("Failed to open Audio Device: %s\n", SDL_GetError());
    return 1;
  } else {
    printf("Opened Audio Device: %d\n", spec.format);
  }

  SDL_PauseAudio(0);

  while (true)
  {
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
      switch (e.type)
      {
      case SDL_QUIT:
        printf("exiting...\n");
        return 0;
      }
    }
  }

  return 0;
}
