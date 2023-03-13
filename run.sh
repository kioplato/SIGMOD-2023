#!/bin/bash

N_CLUSTERS=$1

# The absolute path to this script.
ABSOLUTE_PATH=$(readline -f "$0")
# Project's root directory (this script lives in the project's root).
DIRECTORY=$(dirname "$ABSOLUTE_PATH")

# Compile the project.
(cd "$DIRECTORY" || exit 1; make clean; make -j8)

# Run the solution.
./Build/knng datasets/dummy-data.bin "$N_CLUSTERS"
