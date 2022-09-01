#!/bin/sh
clang -o build/std_math_gen tools/std_math_gen.c
if test $? -ne 0; then
	exit
fi
./build/std_math_gen
