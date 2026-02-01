#!/usr/bin/env sh
set -e

CC="gcc"
CFLAGS="-o robots -g -std=c99 -Iraylib/src/ -Isrc/"
LFLAGS=""
SOURCES="src/main.c src/rendering.c src/common.c src/lang.c src/audio.c src/render_test.c ./raylib/src/libraylib.a"
# Options
RUN_MODE=""

# Parse arguments
for arg in "$@"
do
	case $arg in
		--test|-t)
			CFLAGS="$CFLAGS -DRENDER_TEST"
			echo "Building with RENDER_TEST enabled"
			;;
		--leaks|-l)
			RUN_MODE="leaks"
			echo "Will run with leaks detection"
			;;
		--gdb|-g)
			RUN_MODE="gdb"
			echo "Will run with GDB"
			;;
		--debug|-d)
			CFLAGS="$CFLAGS -DDEBUG_GAME"
			echo "Building with DEBUG_GAME enabled"
			;;
	esac
done

# Add OS-specific flags
os=$(uname)
if [ "$os" = "Linux" ]
then
	LFLAGS="$LFLAGS -lGL -lm -lpthread -ldl -lrt -lX11"
elif [ "$os" = "Darwin" ]
then
	CC="clang"
	LFLAGS="$LFLAGS -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -framework CoreAudio -framework AudioToolbox"
fi

# Compile
$CC $CFLAGS $SOURCES $LFLAGS

# Run
case $RUN_MODE in
	"")
		./robots
		;;
	"leaks")
		if [ "$os" = "Linux" ]
		then
			valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s ./robots
		elif [ "$os" = "Darwin" ]
		then
			leaks --atExit -- ./robots
		fi
		;;
	"gdb")
		gdb -q -ex=r --args ./robots
		;;
	*)
		echo "unknown run mode '$RUN_MODE'"
		;;
esac
