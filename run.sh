#!/usr/bin/env sh
set -e

# Optional flags
CFLAGS=""

# Enable render test mode with: ./run.sh --test
if [ "$1" = "--test" ] || [ "$1" = "-t" ]
then
	CFLAGS="$CFLAGS -DRENDER_TEST"
	echo "Building with RENDER_TEST enabled"
fi

SOURCES="src/main.c src/rendering.c src/common.c src/audio.c src/render_test.c"

os=$(uname)
if [ "$os" = "Linux" ]
then
	gcc -std=c99 $CFLAGS -Iraylib/src/ -Isrc/ $SOURCES ./raylib/src/libraylib.a \
		-lGL -lm -lpthread -ldl -lrt -lX11 \
		-o robots
elif [ "$os" = "Darwin" ]
then
	clang -std=c99 $CFLAGS -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -framework CoreAudio -framework AudioToolbox \
		-o robots -Iraylib/src/ -Isrc/ $SOURCES ./raylib/src/libraylib.a
fi

./robots
