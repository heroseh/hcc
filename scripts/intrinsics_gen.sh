#!/bin/sh
mkdir -p build
clang -o build/intrinsics_gen tools/intrinsics_gen.c
if test $? -ne 0; then
	exit
fi
./build/intrinsics_gen
