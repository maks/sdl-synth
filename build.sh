#!/bin/sh

set -xe 

export CFLAGS="-Wall -Werror -std=c++20 -pedantic `pkg-config sdl2 -cflags` "
export LIBS="`pkg-config sdl2 -libs` -lm"

cc $CFLAGS -o sdlsynth synth.cpp $LIBS