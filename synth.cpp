#include <cstdio>
#include <cassert>

#include <SDL2/SDL.h>
#include "SDL2/SDL_audio.h"


const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 4096;


// oscillator oscillate(float rate, float volume) {
//   oscillator o = {
//       .current_step = 0,
//       .volume = volume,
//       .step_size = (2 * M_PI) / rate,
//   };
//   return o;
// }

void oscillator_callback(void *userdata, Uint8 *stream, int len) {
  assert(false);
}

int main(int argc, char const *argv[])
{
  printf("sdl synth\n");

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
    printf("Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  SDL_AudioSpec spec = {
      .freq = SAMPLE_RATE,
      .format = AUDIO_U16,
      .channels = 1,
      .samples = 4096,
      .callback = oscillator_callback,
  };

  if (SDL_OpenAudio(&spec, NULL) < 0) {
    printf("Failed to open Audio Device: %s\n", SDL_GetError());
    return 1;
  }

  SDL_PauseAudio(0);

  while (true) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_QUIT:
        return 0;
      }
    }
  }



  return 0;
}
