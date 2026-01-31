#!/usr/bin/env sh
set -e

os=$(uname)
if [ "$os" = "Linux" ]
then
	gcc -Iraylib/src/ -Isrc/ src/main.c ./raylib/src/libraylib.a \
		-lGL -lm -lpthread -ldl -lrt -lX11 \
		-o robots
elif [ "$os" = "Darwin" ]
then
	clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL \
		-o robots -Iraylib/src/ -Isrc/ src/main.c ./raylib/src/libraylib.a
fi

./robots
