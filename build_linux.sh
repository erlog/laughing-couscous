#!/bin/sh
g++ renderer.cpp -DLINUX_COMPILE -o renderer -I/usr/include/SDL2 -I/usr/include/x86_64-linux-gnu/ruby-2.1.0 -I/usr/include/ruby-2.1.0 -lm -lruby-2.1 -lSDL2 -lGL -lGLEW -lassimp -Wall -Wno-unused-variable
