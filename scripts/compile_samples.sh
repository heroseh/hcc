#!/bin/sh
./build/hcc -I . -fi samples/triangle.c -fo samples/triangle.spirv && \
./build/hcc -I . -fi samples/alt-2.5d-rgb-color-picker.c -fo samples/alt-2.5d-rgb-color-picker.spirv && \
./build/hcc -I . -fi samples/blob-vacation.c -fo samples/blob-vacation.spirv
