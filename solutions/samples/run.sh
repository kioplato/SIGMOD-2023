#!/bin/bash

DATASET_PATH=$1
SAMPLE_SIZE=$2

make clean
make
./baseline "$DATASET_PATH" "$SAMPLE_SIZE"
