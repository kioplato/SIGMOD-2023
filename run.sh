#!/bin/bash

cd Build
make clean
make -j8
cd ..
./Build/knng datasets/dummy-data.bin 8
