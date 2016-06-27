CFLAGS = -std=c++11 -ggdb -Wall -Wno-unused-variable -Wno-write-strings
SRC = main.cpp
LNK = main

all:
	make linux

linux:
	g++ $(CFLAGS) $(SRC) -DLINUX_COMPILE -o $(LNK) -isystem"/usr/include/SDL2" -isystem"/usr/include/x86_64-linux-gnu/ruby-2.1.0" -isystem"/usr/include/ruby-2.1.0" -lm -lruby-2.1 -lSDL2 -lGL -lGLEW -lassimp

mac:
	g++ $(CFLAGS) $(SRC) -DMAC_COMPILE -o $(LNK) -isystem"/System/Library/Frameworks/Ruby.framework/Versions/2.0/Headers" -isystem"/Library/Frameworks/SDL2.framework/Versions/A/Headers" -isystem"/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers" -isystem"/usr/local/Cellar/glm/0.9.7.4/include" -lruby -lSDL2 -lassimp -lGLEW -framework OpenGL

clean:
	rm $(LNK)
