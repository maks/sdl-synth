#!/bin/sh

export CFLAGS="-Wall -Werror `pkg-config sdl2 -cflags` "
export LIBS=`pkg-config sdl2 -libs`

cc $CFLAGS -o sdlsynth synth.cpp $LIBS