#!/usr/bin/env sh
set -e

os=$(uname)
if [ "$os" = "Linux" ]
then
	gcc -std=c99 -Iraylib/src/ -Isrc/ src/main.c src/rendering.c src/common.c src/audio.c ./raylib/src/libraylib.a \
		-lGL -lm -lpthread -ldl -lrt -lX11 \
		-o robots
elif [ "$os" = "Darwin" ]
then
	clang -std=c99 -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -framework CoreAudio -framework AudioToolbox \
		-o robots -Iraylib/src/ -Isrc/ src/main.c src/rendering.c src/common.c src/audio.c ./raylib/src/libraylib.a
fi

./robots
