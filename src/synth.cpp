#include <cstdio>
#include <cassert>

#include "SDL2/SDL_audio.h"

#include <SDL2/SDL.h>

#include "picosynth/picosynth.h"

void oscillator_callback(void *userdata, uint8_t *byteStream, int len) {

  generateWaves(byteStream);
  update_envelopes();
}

static void handle_note_keys(SDL_Keysym *keysym) {
  /* change note or octave depending on which key is pressed */
  int new_note = get_note();
  switch (keysym->sym) {
  case SDLK_a:
    new_note = 57;
    break;
  case SDLK_s:
    new_note = 58;
    break;
  case SDLK_d:
    new_note = 59;
    break;
  case SDLK_f:
    new_note = 60;
    break;
  case SDLK_g:
    new_note = 61;
    break;
  case SDLK_h:
    new_note = 62;
    break;
  case SDLK_j:
    new_note = 63;
    break;
  case SDLK_k:
    new_note = 64;
    break;
  }
  printf("new note:%d\n", new_note);
  set_note(new_note);
}

static void handle_key_down(SDL_Keysym *keysym) { handle_note_keys(keysym); }

int main(int argc, char const *argv[]) {
  printf("sdl synth\n");

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
    printf("Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  // SDL WINDOW setup

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_GLContext context;

  window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            640, 480, SDL_WINDOW_OPENGL);

  if (window != NULL) {
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
      printf("\nFailed to create context: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer != NULL) {
      SDL_GL_SetSwapInterval(1);
      SDL_SetWindowTitle(window, "SDL2 synth sample 2");
    } else {
      printf("Failed to create renderer: %s", SDL_GetError());
    }
  } else {
    printf("Failed to create window:%s", SDL_GetError());
  }

  // SDL window setup done ==========

  SDL_AudioSpec spec = {
      .freq = SAMPLE_RATE,
      .format = AUDIO_S16LSB,
      .channels = 1,
      .samples = BUFFER_SIZE,
      .callback = oscillator_callback,
  };

  if (SDL_OpenAudio(&spec, NULL) < 0) {
    printf("Failed to open Audio Device: %s\n", SDL_GetError());
    return 1;
  } else {
    printf("Opened Audio Device: %d\n", spec.format);
  }

  // set picosynth defaults
  set_defaults();

  SDL_PauseAudio(0);

  while (true) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_KEYDOWN:
        // dont want key repeats
        if (e.key.repeat == 0) {
          handle_key_down(&e.key.keysym);
          envelope_gate(1);
          printf("NOTE FREQ:%f\n", noteToFreq(get_note()));
        }
        break;
      case SDL_KEYUP:
        printf("STOP NOTE");
        envelope_gate(0);
        break;
      case SDL_QUIT:
        printf("exiting...\n");
        return 0;
      }
    }
  }

  return 0;
}
