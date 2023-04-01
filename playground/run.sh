#!/bin/sh

clang -pedantic -D_GNU_SOURCE -I../libhmaths -I../libhccintrinsics -I../interop -lm -std=gnu11 -Werror -Wfloat-conversion -Wextra -lX11 -lvulkan -g -o ./playground ./app/main.c
if [ $? -ne 0 ]; then
	exit
fi

./playground

