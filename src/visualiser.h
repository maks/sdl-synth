#include "SDL2/SDL_audio.h"

#include <SDL2/SDL.h>

const int VIS_FRAMES = 4;

bool visualiser_update(SDL_Renderer *renderer, uint8_t *audio_buffer,
                       int16_t buffer_length, int window_height,
                       int window_width);