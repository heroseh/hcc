#!/bin/sh
mkdir -p build
clang -pedantic -D_GNU_SOURCE -lm -std=gnu11 -Werror -Wfloat-conversion -Wimplicit-fallthrough -Wextra -g -o build/hcc src/hcc_main.c
