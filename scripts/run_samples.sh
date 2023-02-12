#!/bin/sh
mkdir -p build
clang -pedantic -D_GNU_SOURCE -I./ -lm -std=gnu11 -Werror -Wfloat-conversion -Wimplicit-fallthrough -Wextra -lX11 -lvulkan -g -o build/samples samples/app/main.c

if [ $? -ne 0 ]; then
	exit
fi

./build/samples

