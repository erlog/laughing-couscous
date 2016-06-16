all:
	g++ renderer.cpp -DLINUX_COMPILE -o renderer -std=c++11 -ggdb -I/usr/include/SDL2 -I/usr/include/x86_64-linux-gnu/ruby-2.1.0 -I/usr/include/ruby-2.1.0 -I/usr/include/freetype2 -lfreetype -lm -lruby-2.1 -lSDL2 -lGL -lGLEW -lassimp -Wall -Wno-unused-variable

clean:
	rm -rf renderer
