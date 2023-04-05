#!/bin/bash

rm -rf Build
cmake -BBuild
cd Build || echo "Failed to cd into Build/ dir."
make -j32
./knng --dataset ../dummy-data.bin --output output.bin --n-clusters 500 --n-iters 1 --n-nearest-clusters 50
