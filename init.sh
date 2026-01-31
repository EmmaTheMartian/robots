#!/usr/bin/env sh
set -e

RAYLIB_VERSION=5.5

if [ ! -e "raylib.tar.gz" ]
then
	curl -Lo raylib.tar.gz https://github.com/raysan5/raylib/archive/refs/tags/$RAYLIB_VERSION.tar.gz
fi

if [ ! -e "raylib/" ]
then
	tar -xkf raylib.tar.gz && mv -v raylib-$RAYLIB_VERSION raylib
fi

cd raylib/src
make
cd ../..
