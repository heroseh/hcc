#!/bin/sh
./scripts/build.sh
cd samples
export HCC_EXE_DIR="../build/"
./run_samples.sh

