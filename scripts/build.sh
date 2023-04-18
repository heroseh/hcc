#!/bin/sh

FLAGS="-pedantic -D_GNU_SOURCE -std=gnu11 -Werror -Wfloat-conversion -Wimplicit-fallthrough -Wextra -g -lm -pthread"
if [ "${1}" = "release" ]; then
	FLAGS="$FLAGS -O2"
fi

mkdir -p build

cd build
ln -snf ../libc libc
ln -snf ../libhmaths libhmaths
ln -snf ../libhccintrinsics libhccintrinsics
cd ..\

clang $FLAGS -o build/hcc src/hcc_main.c && \
build/hcc -O -fi samples/shaders.c -fo samples/shaders.spirv -fomc samples/shaders-metadata.h && \
clang -pedantic -D_GNU_SOURCE -Ilibhmaths -Ilibhccintrinsics -Iinterop -lm -std=gnu11 -Werror -Wfloat-conversion -Wextra -lX11 -lvulkan -g -o ./samples/samples ./samples/app/main.c && \
clang -pedantic -D_GNU_SOURCE -Ilibhmaths -Ilibhccintrinsics -Iinterop -lm -std=gnu11 -Werror -Wfloat-conversion -Wextra -lX11 -lvulkan -g -o ./playground/playground ./playground/app/main.c

EXIT_CODE=$?
if [ $EXIT_CODE -ne 0 ]; then
	exit $EXIT_CODE
fi

if [ "${1}" = "release" ]; then
	echo "=========== Building Release Package ==========="
	cd build
	tar -cvzf "hcc-0.0.1-linux.tar.gz" hcc ../libc ../libhmaths ../libhccintrinsics ../interop ../samples ../playground ../docs ../README.md ../LICENSE
fi

