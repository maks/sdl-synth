#include <cstdio>
#include <cassert>

#include "ADSR.h"
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
 * The current wave and env settings. Dont define here to save space.
 */
uint16_t env[HARMONICS][5];
char wave[HARMONICS][2];

/*
 * The current envelope settings. This represents the volume of the harmonic,
 * and the state it is in.
 */
int filt[HARMONICS] = {0, 0, 0, 0, 0, 0};
u_char filt_state[HARMONICS] = {0, 0, 0, 0, 0, 0};

float phase_int[HARMONICS] = {0, 0, 0, 0, 0, 0};

ADSR adsr[HARMONICS] = {ADSR(), ADSR(), ADSR(), ADSR(), ADSR(), ADSR()};

u_char gate = 0;

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

  /* cast buffer as 16bit signed int */
  s_byte_stream = (Sint16 *)byte_stream;

  // generate samples
  for (int i = 0; i < BUFFER_SIZE; i++) {
    int fullResult = 0;
    for (int h = 0; h < HARMONICS; h++) {
      fullResult +=
          generatePhaseSample(phase_increment[h], phase_int[h], filt[h]);
    }
    // write sum of all harmonics into audio buffer
    s_byte_stream[i] = fullResult;
  }
}

void update_envelopes() {
  //
  for (int h = 0; h < HARMONICS; h++) {
    filt[h] = (adsr[h].process() * 5000);
  }
}

const int CONTROL_RATE = SAMPLE_RATE / 256;
void set_defaults() {
  // initialize settings
  for (int h = 0; h < HARMONICS; h++) {
    adsr[h].setAttackRate(.3 * CONTROL_RATE); // .1 second
    adsr[h].setDecayRate(.3 * CONTROL_RATE);
    adsr[h].setReleaseRate(1 * CONTROL_RATE);
    adsr[h].setSustainLevel(.8);
  }
}

void adsr_gate(bool on) {
  for (int h = 0; h < HARMONICS; h++) {
    adsr[h].gate(on);
  }
}

/*
 * Called every ~10ms, this updates the individual harmonic volumes in line with
 * their Envelope. There are 4 envelope types. These are ADSR, delayed ADSR,
 * AD repeat and delayed AD repeat. The delayed envelope waits to go into the
 * decay phase until all active oscillators have finished their attack phase.
 *
 * State 0 => attack
 * State 1 => delay
 * State 2 => decay
 * State 3 => sustain
 * State 4 => release
 * State 5 => note off
 *
 * A standard adsr will go, states: 0, 2, 3, 4, 5
 */
// void update_envelopes() {
//   u_char finished = 0;
//   u_char playing = 6;
//   int attack, decay, level, sustain, rel;
//   u_char etype;
//   u_char tremolo = 1;

//   /*
//    * Work out which notes are still playing, and which have
//    * finished their attack phase. Repeating envelopes don't
//    * count towards a playing note as they repeat indefinitely.
//    */
//   for (char i = 0; i < HARMONICS; i++) {
//     if (env[i][0] < 2 && wave[i][0])
//       // tremolo = 0;
//       if (filt_state[i] > 0 || env[i][0] > 1) {
//         finished++;
//         if (filt_state[i] == 5 || env[i][0] > 1)
//           playing--;
//       }
//   }

//   /*
//    * Loop through the oscillators, and set their filt value. The
//    * filt value represents the current volume of the oscillator.
//    */
//   for (u_char i = 0; i < HARMONICS; i++) {
//     level = ((int)wave[i][0]) << 8;
//     if (level == 0) {
//       filt_state[i] = 6;
//       continue;
//     }
//     etype = (u_char)(env[i][0]);
//     attack = env[i][1];
//     decay = env[i][2];
//     sustain = (level >> 8) * env[i][3];
//     rel = env[i][4];
//     switch (filt_state[i]) {
//     case 0: // Attack
//       filt[i] += attack;
//       if (attack == 0 || filt[i] >= level) {
//         filt[i] = level;
//         filt_state[i] = (etype & 1) ? 1 : 2;
//       }
//       break;
//     case 1: // Delay
//       if (finished == 6)
//         filt_state[i] = 2;
//       else
//         break;
//     case 2: // Decay
//       filt[i] -= decay;
//       if (decay == 0 || filt[i] <= sustain) {
//         filt[i] = sustain;
//         if (etype > 1 && etype < 4) {
//           if (playing || tremolo)
//             filt_state[i] = 0;
//           else
//             filt_state[i] = 6;
//         } else {
//           filt_state[i] = 3;
//         }
//       }
//       break;
//     case 3: // Sustain stay here until told to release
//       if (filt[i] == 0)
//         filt_state[i] = 6;
//       break;
//     case 4: // Release
//       if (etype == 4) {
//         filt[i] += rel;
//         if (rel == 0 || filt[i] >= level) {
//           filt[i] = level;
//           filt_state[i] = 5;
//         }
//       } else {
//         filt[i] -= rel;
//         if (rel == 0 || filt[i] <= 0) {
//           filt[i] = 0;
//           filt_state[i] = 6;
//         }
//       }
//       break;
//     case 5: // Reverse Release
//       filt[i] -= decay;
//       if (rel == 0 || filt[i] <= 0) {
//         filt[i] = 0;
//         filt_state[i] = 6;
//       }
//       break;
//     default:       // Note off
//       filt[i] = 0; // take care of repeating oscillators
//       break;
//     }
//   }
// }

// void set_defaults() {
//   memset(wave, 0, sizeof(wave));
//   memset(env, 0, sizeof(env));
//   // memset(lfo, 0, sizeof(lfo));
//   // memset(special, 0, sizeof(special));

//   /*
//    * Simple sine wave, max vol, instant attack, max sustain, quick release
//    */
//   // wave[h][0] is oscillator amplitude
//   wave[0][0] = 60;
//   // env[h][0] is envelope type
//   // env[h][1] is envelope attack
//   // env[h][2] is envelope decay
//   // env[h][3] is envelope sustain
//   // env[h][4] is envelope release

//   env[0][1] = 40;
//   env[0][2] = 40;
//   env[0][3] = 190;
//   // env[0][2] = 240;
//   env[0][3] = env[1][3] = env[2][3] = env[3][3] = env[4][3] = env[5][3] =
//   255;
// }

//=== PICO SYNTH
char callback_counter = 0;
void oscillator_callback(void *userdata, Uint8 *byteStream, int len) {

  generateWaves(byteStream);
  update_envelopes();
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

  // set picosynth defaults
  set_defaults();

  SDL_PauseAudio(0);

  while (true) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_KEYDOWN:
        handle_key_down(&e.key.keysym);
        adsr_gate(true);
        printf("NOTE FREQ:%f\n", get_pitch(note));
        break;
      case SDL_KEYUP:
        printf("STOP NOTE");
        adsr_gate(false);
        break;
      case SDL_QUIT:
        printf("exiting...\n");
        return 0;
      }
    }
  }

  return 0;
}
