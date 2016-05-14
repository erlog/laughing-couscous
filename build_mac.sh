#!/bin/sh
RUBY="/System/Library/Frameworks/Ruby.framework/Versions/2.0/Headers"
SDL="/Library/Frameworks/SDL2.framework/Versions/A/Headers"
OpenGL="/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
GLM="/usr/local/Cellar/glm/0.9.7.4/include"
g++ renderer.cpp -o renderer -isystem"$RUBY" -isystem"$GLM" -isystem"$SDL" -lruby -lSDL2 -lGLEW -framework OpenGL -Wall -Wno-unused-variable


