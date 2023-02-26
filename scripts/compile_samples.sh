#!/bin/sh
./build/hcc -I ./shader-include -I ./libc-gpu -fi samples/triangle.c -fi shader-include/hmaths/maths.c -fo samples/triangle.spirv && \
./build/hcc -I ./shader-include -I ./libc-gpu -fi samples/compute-square.c -fi shader-include/hmaths/maths.c -fo samples/compute-square.spirv && \
./build/hcc -I ./shader-include -I ./libc-gpu -fi samples/texture.c -fi shader-include/hmaths/maths.c -fo samples/texture.spirv && \
./build/hcc -I ./shader-include -I ./libc-gpu -fi samples/alt-2.5d-rgb-color-picker.c -fi shader-include/hmaths/maths.c -fo samples/alt-2.5d-rgb-color-picker.spirv && \
./build/hcc -I ./shader-include -I ./libc-gpu -fi samples/blob-vacation.c -fi shader-include/hmaths/maths.c -fo samples/blob-vacation.spirv

