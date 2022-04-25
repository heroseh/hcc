#!/bin/sh

clang -Werror -Wno-format-security -Wfloat-conversion -Wextra -g -o build/hcc src/hcc_main.c

