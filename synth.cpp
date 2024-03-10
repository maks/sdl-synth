#include <cstdio>
#include <cassert>

#include <SDL2/SDL.h>
#include "SDL2/SDL_audio.h"

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 4096;

// https://blog.fredrb.com/2023/08/08/audio-programming-note-sdl/
typedef struct {
  float current_step;
  float step_size;
  float volume;
} oscillator;

oscillator oscillate(float rate, float volume) {
  oscillator o = {
      .current_step = 0,
      .step_size = (float) (2 * M_PI) / rate,
      .volume = volume,
  };
  return o;
}

float next(oscillator *os) {
  os->current_step += os->step_size;
  return sinf(os->current_step) * os->volume;
}

float A4_freq = (float)SAMPLE_RATE / 440.00f;
auto a4 = oscillate(A4_freq, 0.8f);

void oscillator_callback(void *userdata, Uint8 *stream, int len) {
  float *fstream = (float *)stream;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    float v = next(&a4);
    fstream[i] = v;
  }
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
      .format = AUDIO_F32,
      .channels = 1,
      .samples = BUFFER_SIZE,
      .callback = oscillator_callback,
  };

  if (SDL_OpenAudio(&spec, NULL) < 0)
  {
    printf("Failed to open Audio Device: %s\n", SDL_GetError());
    return 1;
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
        return 0;
      }
    }
  }

  return 0;
}
