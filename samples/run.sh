#!/bin/sh

../hcc -O -fi shaders.c -fo shaders.spirv -fomc shaders-metadata.h && \
clang -pedantic -D_GNU_SOURCE -I../libhmaths -I../libhccintrinsics -I../interop -lm -std=gnu11 -Werror -Wfloat-conversion -Wextra -lX11 -lvulkan -g -o ./samples ./app/main.c

if [ $? -ne 0 ]; then
	exit
fi

./samples

