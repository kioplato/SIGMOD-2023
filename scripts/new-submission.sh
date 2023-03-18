#!/bin/bash

# Purpose: create a new ReproZip submission using the specified hyperpameters.
# The output is a ready-to-submit submission.rpz file. Just submit it upstream.

# The name of the script.
SCRIPT_BASENAME=$(basename "$0")

function die()
{
	# String to print to stdout.
	str=$1

	echo "[$SCRIPT_BASENAME] fatal: $str"
	echo "run $SCRIPT_BASENAME --help for usage."

	exit 1
}

function usage()
{
	echo "Usage:"
	echo " Prepare a submission.rpz to submit upstream."
	echo
	echo " You need to specify (not necessarily in this order):"
	echo " --dataset <path>: Specify the dataset to use for the submission."
	echo " --n-clusters <unsigned>: The number of clusters to use."
	echo " --n-iters <unsigned>: The maximum number of iterations."
	echo " --search-depth <unsigned>: How many nearest clusters to search into."
}

arguments=("$@")
for ((i=0; i < ${#arguments[@]}; ++i))
do
	case ${arguments[$i]} in
		"--help")
			usage
			exit
			;;
		"--dataset")
			((++i))
			# The path to the dataset to use.
			DATASET_PATH=${arguments[$i]}
			;;
		"--n-clusters")
			((++i))
			# The number of clusters to create.
			NUM_CLUSTERS=${arguments[$i]}
			;;
		"--n-iters")
			((++i))
			# The number of iterations to perform.
			NUM_ITERS=${arguments[$i]}
			;;
		"--search-depth")
			((++i))
			# The number of nearest clusters to search into.
			SEARCH_DEPTH=${arguments[$i]}
			;;
		*)
			die "flag ${arguments[$i]} not recognized."
	esac
done

# Dataset path must be set.
if [ -z "$DATASET_PATH" ]
then
	die "--dataset <path> not provided."
else
	# The dataset path must exist.
	if [ ! -e "$DATASET_PATH" ]
	then
		die "specified --dataset $DATASET_PATH does not exist."
	# The dataset path must be a regular file.
	elif [ ! -f "$DATASET_PATH" ]
	then
		die "specified --dataset $DATASET_PATH isn't a regular file."
	fi
fi

# Number of clusters to use must be set.
if [ -z "$NUM_CLUSTERS" ]
then
	die "--n-clusters <unsigned> not provided."
# The number of clusters must be a positive number.
elif ! [[ $NUM_CLUSTERS =~ [1-9]+[0-9]* ]]
then
	die "--n-clusters <unsigned> is not a positive number."
fi

# Maximum number of iterations to perform must be set.
if [ -z "$NUM_ITERS" ]
then
	die "--n-iters <unsigned> not provided."
# The number of iterations must be a positive number.
elif ! [[ $NUM_ITERS =~ [1-9]+[0-9]* ]]
then
	die "--n-iters <unsigned> is not a positive number."
fi

# The number of nearest clusters to search into must be set.
if [ -z "$SEARCH_DEPTH" ]
then
	die "--search-depth <unsigned> not provided."
# The number of nearest clusters to search into must be a positive number.
elif ! [[ $NUM_ITERS =~ [1-9]+[0-9]* ]]
then
	die "--search-depth <unsigned> is not a positive number."
fi

# The absolute path to this script.
SCRIPT_ABSOLUTE_PATH=$(readline -f "$0")
# This script's directory (this script lives in the scripts/ directory).
SCRIPT_DIRECTORY=$(dirname "$SCRIPT_ABSOLUTE_PATH")
# Project's root directory.
ROOT_DIR=$(readlink -f "$SCRIPT_DIRECTORY/..")

# Where to build the project.
BUILD_DIR="/tmp/SIGMOD-SUBMISSION-Build"

# The number of threads to use for compilation.
N_PROCS=$(nproc)

# Compile the project in subshell.
(
	# Go into the project's root directory.
	cd "$ROOT_DIR" || diei "could not cd $ROOT_DIR"
	# Generate the makefile.
	cmake -B"$BUILD_DIR" > /dev/null 2>&1 || diei "failed to cmake -BBuild"
	# cd into Build/ directory.
	cd "$BUILD_DIR" || diei "could not cd $BUILD_DIR"
	# Compile the project.
	make -j"$N_PROCS" > /dev/null 2>&1 || diei "could not make -j8"
)

# Find the executable's name from CMakeLists.txt file.
EXE_BASENAME=$(grep -o "project([a-zA-Z0-9+]*)" "$ROOT_DIR/CMakeLists.txt" | grep -oP "project\(\K.*?(?=\))")
# The executable's path.
EXE_PATH="$BUILD_DIR/$EXE_BASENAME"

# Where to store the reprozip trace files.
TRACE_DIR="$BUILD_DIR/.reprozip-trace"

# Trace the execution.
reprozip -d "$TRACE_DIR" trace "$EXE_PATH --dataset $DATASET_PATH\
	--n-clusters $NUM_CLUSTERS --n-iters $NUM_ITERS --search-depth $SEARCH_DEPTH"
# Pack the traced execution.
reprozip -d "$TRACE_DIR" pack "submission-${NUM_CLUSTERS}clusters-${NUM_ITERS}iters-${SEARCH_DEPTH}depth.rpz"

# Clean the /tmp/ directory.
rm -rf "$BUILD_DIR"
