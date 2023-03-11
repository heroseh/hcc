#!/bin/sh
mkdir -p build
./build/hcc -O -I ./shader-include -I ./libc-gpu -fi samples/shaders.c -fi shader-include/hmaths/maths.c -fo samples/shaders.spirv -fomc samples/shaders-metadata.h && \
clang -pedantic -D_GNU_SOURCE -I./shader-include -I./interop -lm -std=gnu11 -Werror -Wfloat-conversion -Wextra -lX11 -lvulkan -g -o build/samples samples/app/main.c

if [ $? -ne 0 ]; then
	exit
fi

./build/samples

