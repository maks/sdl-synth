# Testbed for synths using SDL2

This is just a test application for synth engines to make it easier for me to prepare to integrate them as [picoTracker instruments](https://github.com/democloid/picoTracker/).

## prereqs

```
sudo apt install libsdl2-dev
```

## build

```
mkdir build
cd build
cmake ..
make
./sdlsynth
```

Currently only tested to build and run on Ubuntu 23.10.

## Tinysynth

This is a port of the additive synth: [AT-tiny85-additive-synth](https://github.com/jsvader/AT-tiny85-additive-synth)

## Tinysynth2

This is additive synth inspired AT-tiny85-additive-synth but mostly built from scratch.

## References

Very useful set of articles about wavetable oscillator, ADSR implementations, etc:
https://www.earlevel.com/main/2012/05/25/a-wavetable-oscillator-the-code/

SDL2 Audio Synth samples:
https://github.com/lundstroem/synth-samples-sdl2/tree/master/src


TODO