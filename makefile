all:
	make mac

linux:
	g++ renderer.cpp -DLINUX_COMPILE -o renderer -std=c++11 -ggdb -I/usr/include/SDL2 -I/usr/include/x86_64-linux-gnu/ruby-2.1.0 -I/usr/include/ruby-2.1.0 -I/usr/include/freetype2 -lfreetype -lm -lruby-2.1 -lSDL2 -lGL -lGLEW -lassimp -Wall -Wno-unused-variable

mac:
	g++ renderer.cpp -DMAC_COMPILE -o renderer -isystem"/System/Library/Frameworks/Ruby.framework/Versions/2.0/Headers" -isystem"/Library/Frameworks/SDL2.framework/Versions/A/Headers" -isystem"/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers" -isystem"/usr/local/Cellar/glm/0.9.7.4/include" -lruby -lSDL2 -lassimp -lGLEW -framework OpenGL -Wall -Wno-unused-variable

clean:
	rm -rf renderer
