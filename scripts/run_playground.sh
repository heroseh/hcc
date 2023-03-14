#!/bin/sh
./scripts/build.sh
cd playground
export HCC_EXE_DIR="../build/"
./run.sh
