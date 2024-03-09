#include <cstdio>

#include <SDL2/SDL.h>
#include "SDL2/SDL_audio.h"

int main(int argc, char const *argv[])
{
  printf("sdl synth\n");

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
    printf("Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }



  return 0;
}
