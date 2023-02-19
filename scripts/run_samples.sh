#!/bin/sh
./scripts/compile_samples.sh
mkdir -p build
clang -pedantic -D_GNU_SOURCE -I./shader-include -lm -std=gnu11 -Werror -Wfloat-conversion -Wextra -lX11 -lvulkan -g -o build/samples samples/app/main.c

if [ $? -ne 0 ]; then
	exit
fi

./build/samples

