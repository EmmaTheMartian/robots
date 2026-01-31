#!/usr/bin/env sh
set -e

os=$(uname)
if [ "$os" = "Linux" ]
then
	gcc -Iraylib/src/ -Isrc/ src/*.c ./raylib/src/libraylib.a \
		-lGL -lm -lpthread -ldl -lrt -lX11 \
		-g -o robots
elif [ "$os" = "Darwin" ]
then
	clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL \
		-g -o robots -Iraylib/src/ -Isrc/ src/*.c ./raylib/src/libraylib.a
fi

# gdb -q -ex=r --args ./robots
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all -s ./robots
