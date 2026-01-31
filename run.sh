#!/usr/bin/env sh
set -e

# Optional flags
CFLAGS=""
USE_LEAKS=false

# Parse arguments
for arg in "$@"
do
	case $arg in
		--test|-t)
			CFLAGS="$CFLAGS -DRENDER_TEST"
			echo "Building with RENDER_TEST enabled"
			;;
		--leaks|-l)
			USE_LEAKS=true
			echo "Will run with leaks detection"
			;;
	esac
done

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

if [ "$USE_LEAKS" = true ]
then
	leaks --atExit -- ./robots
else
	./robots
fi
