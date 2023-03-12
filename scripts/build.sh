#!/bin/sh

FLAGS="-pedantic -D_GNU_SOURCE -lm -std=gnu11 -Werror -Wfloat-conversion -Wimplicit-fallthrough -Wextra -g"
if [[ $1 == "release" ]]; then
	FLAGS="$FLAGS -O2"
fi

mkdir -p build
clang $FLAGS -o build/hcc src/hcc_main.c

cd build
ln -snf ../libc libc
ln -snf ../libhmaths libhmaths
ln -snf ../libhccintrinsics libhccintrinsics

if [[ $1 == "release" ]]; then
	echo "=========== Building Release Package ==========="
	tar -cvzf "hcc-0.0.1-linux.tar.gz" hcc ../libc ../libhmaths ../libhccintrinsics ../interop ../samples ../docs ../README.md ../LICENSE
fi

