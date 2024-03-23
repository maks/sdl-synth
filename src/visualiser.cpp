#include <vector>

#include <SDL2/SDL_render.h>

#include "visualiser.h"

void visialiser_draw_wave(SDL_Renderer *renderer, const SDL_Point &start,
                          int length, int pixel_amplitude,
                          const SDL_Color &color, int16_t buffer_length,
                          uint8_t *audio_buffer) {
  std::vector<SDL_Point> points;

  int16_t *data = (int16_t *)audio_buffer;
  for (int i = 0; i < (buffer_length); i++) {
    int16_t amplitude = data[i];
    points.push_back(SDL_Point{start.x + i, (start.y - (amplitude / 16))});
  }

  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawPoints(renderer, &points[0], points.size());
}

bool visualiser_update(SDL_Renderer *renderer, uint8_t *audio_buffer,
                       int16_t buffer_length, int window_height,
                       int window_width) {
  // Render visualizer
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
  visialiser_draw_wave(renderer, SDL_Point{0, window_height / 2}, window_width,
                       window_height, SDL_Color{255, 255, 30, 255},
                       buffer_length, audio_buffer);
  SDL_RenderPresent(renderer);

  return true;
}