#include <math.h>
#include <stdint.h>
#include <stdio.h>

static int8_t *sine_wave_table;

static const double pi = 3.14159265358979323846;

// ref:
// https://github.com/lundstroem/synth-samples-sdl2/blob/master/src/synth_samples_sdl2_2.c
static void build_sine_table(int8_t *data, int wave_length) {
  int i;
  /*
      Build sine table to use as oscillator:
      Generate a 16bit signed integer sinewave table with wave_length samples.
      This table will be used to produce the notes.
      Different notes will be created by stepping through
      the table at different intervals (phase).
  */
  double phase_increment = (2.0f * pi) / (double)wave_length;
  double current_phase = 0;
  for (i = 0; i < wave_length; i++) {
    int sample = (int)(sin(current_phase) * INT8_MAX);
    data[i] = (int8_t)sample;
    current_phase += phase_increment;
  }
}

int main(void) {
  int table_length = 256;
  /* allocate memory for sine table and build it */
  sine_wave_table = (int8_t *)malloc(sizeof(int8_t) * table_length);
  build_sine_table(sine_wave_table, table_length);

  printf("{");
  double current_step = 0;
  for (int i = 0; i < table_length; i++) {
    printf("%d,", sine_wave_table[i]);
  }
  printf("}\n");
}