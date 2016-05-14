#!/bin/sh
gcc renderer.c -o renderer -I/usr/include/SDL2 -I/usr/include/x86_64-linux-gnu/ruby-2.1.0 -I/usr/include/ruby-2.1.0 -lm -lruby-2.1 -lSDL2 -lGL -lGLEW -Wall -Wno-unused-variable
