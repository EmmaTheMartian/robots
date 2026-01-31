#!/usr/bin/env sh
set -e

gcc -orobots -Iraylib/src/ src/main.c ./raylib/src/libraylib.a -lm
./robots
