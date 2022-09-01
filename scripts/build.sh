#!/bin/sh
mkdir -p build
clang -pedantic -std=gnu11 -Werror -Wfloat-conversion -Wextra -g -o build/hcc src/hcc_main.c
