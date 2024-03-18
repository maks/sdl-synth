#include <cstdio>
#include <cassert>

#include "SDL2/SDL_audio.h"
#include "fixed.h"
#include <SDL2/SDL.h>

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 256;

static int note = 41; /* integer representing midi notes */

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
float phase_int[HARMONICS] = {0, 0, 0, 0, 0, 0};

int generatePhaseSample(float phase_increment, float &phase_index, int vol) {
  int result = 0;
  phase_index = phase_index;
  phase_index += phase_increment;

  if (phase_index >= LUT_SIZE) {
    int diff = phase_index - LUT_SIZE;
    phase_index = diff;
  }

  result = sine[(int)phase_index];
  int result_fp = i2fp(result);
  auto res_fp = fp_mul(result_fp, vol);
  return fp2i(res_fp);
}

/*
  Calculate pitch from note value.
  offset note by 57 halfnotes to get correct pitch from the range we have chosen
  for the notes.
*/
static const float chromatic_ratio = 1.059463094359295264562;
static float get_pitch(double note) {
  double p = pow(chromatic_ratio, note - 57);
  p *= 440;
  return p;
}

void generateWaves(Uint8 *byte_stream) {
  Sint16 *s_byte_stream;

  // get correct phase increment for note depending on sample rate and LUT
  // length.
  float baseNoteFreq = (get_pitch(note) / SAMPLE_RATE) * LUT_SIZE;

  // Harmonics of primary freq
  float phase_increment[HARMONICS] = {
      baseNoteFreq,     2 * baseNoteFreq, 3 * baseNoteFreq,
      4 * baseNoteFreq, 5 * baseNoteFreq, 6 * baseNoteFreq,
  };

  // max 42038 across all harmonics
  int volume[HARMONICS] = {3000, 3000, 3000, 3000, 3000, 3000};

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

static void handle_note_keys(SDL_Keysym *keysym) {
  /* change note or octave depending on which key is pressed */
  int new_note = note;
  switch (keysym->sym) {
  case SDLK_a:
    new_note = 34;
    break;
  case SDLK_s:
    new_note = 37;
    break;
  case SDLK_d:
    new_note = 41;
    break;
  case SDLK_f:
    new_note = 49;
    break;
  }
  printf("new note:%d\n", new_note);
  note = new_note;
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

  SDL_PauseAudio(0);

  while (true) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_KEYDOWN:
        handle_key_down(&e.key.keysym);
        printf("NOTE FREQ:%f\n", get_pitch(note));
        break;
      case SDL_QUIT:
        printf("exiting...\n");
        return 0;
      }
    }
  }

  return 0;
}
