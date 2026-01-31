#!/usr/bin/env sh
set -e

os=$(uname)
if [ "$os" = "Linux" ]
then
	gcc -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
		-o robots -Iraylib/src/ src/main.c ./raylib/src/libraylib.a
elif [ "$os" = "Darwin" ]
then
	clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL \
		-o robots -Iraylib/src/ src/main.c ./raylib/src/libraylib.a
fi

./robots
