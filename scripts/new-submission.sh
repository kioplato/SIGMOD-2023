#!/bin/bash

# Purpose: create a new ReproZip submission using the specified hyperpameters.
# The output is a ready-to-submit submission.rpz file. Just submit it upstream.

# The absolute path to this script.
_SCRIPT_ABSOLUTE_PATH=$(readlink -f "$0")
# This script's directory (this script lives in the scripts/ directory).
_SCRIPT_DIRECTORY=$(dirname "$_SCRIPT_ABSOLUTE_PATH")
# Project's root directory.
_ROOT_DIR=$(readlink -f "$_SCRIPT_DIRECTORY/..")

source "$_ROOT_DIR/scripts/die.sh"

unset _SCRIPT_ABSOLUTE_PATH
unset _SCRIPT_DIRECTORY

function usage()
{
	echo "Usage:"
	echo " Prepare a submission.rpz to submit upstream."
	echo
	echo " You need to specify (not necessarily in this order):"
	echo " --dataset <path>: Specify the dataset to use for the submission."
	echo " --n-clusters <unsigned>: The number of clusters to use."
	echo " --n-iters <unsigned>: The maximum number of iterations."
	echo " --n-nearest-clusters <unsigned>: How many nearest clusters to search into."
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
		"--n-nearest-clusters")
			((++i))
			# The number of nearest clusters to search into.
			NUM_NEAREST_CLUSTERS=${arguments[$i]}
			;;
		*)
			die "flag ${arguments[$i]} not recognized."
	esac
done

unset arguments

DATASET_PATH=$(readlink -f "$DATASET_PATH")

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
if [ -z "$NUM_NEAREST_CLUSTERS" ]
then
	die "--n-nearest-clusters <unsigned> not provided."
# The number of nearest clusters to search into must be a positive number.
elif ! [[ $NUM_NEAREST_CLUSTERS =~ [1-9]+[0-9]* ]]
then
	die "--n-nearest-clusters <unsigned> is not a positive number."
fi

# Where to build the project.
_BUILD_DIR=$(mktemp -d "/tmp/SIGMOD-SUBMISSION-Build.XXXXXXXXXX")
# The number of threads to use for compilation.
_N_PROCS=$(nproc)

# Compile and run the project.
(
	# Go into the project's root directory.
	cd "$_ROOT_DIR" || diei "could not cd $_ROOT_DIR"
	# Generate the makefile.
	cmake -B"$_BUILD_DIR" > /dev/null 2>&1 || diei "failed to cmake -BBuild"

	# cd into Build/ directory.
	cd "$_BUILD_DIR" || diei "could not cd $_BUILD_DIR"
	# Compile the project.
	make -j"$_N_PROCS" > /dev/null 2>&1 || diei "could not make -j8"

	# Find the executable's name from CMakeLists.txt file.
	EXE_BASENAME=$(grep -o "project([a-zA-Z0-9+]*)" "$_ROOT_DIR/CMakeLists.txt" | grep -oP "project\(\K.*?(?=\))")
	# The executable's path.
	EXE_PATH="$_BUILD_DIR/$EXE_BASENAME"

	$EXE_PATH --dataset "$DATASET_PATH" --n-clusters "$NUM_CLUSTERS" --n-iters "$NUM_ITERS" --n-nearest-clusters "$NUM_NEAREST_CLUSTERS" --output output.bin
)

# Clean the /tmp directory.
rm -rf "$_BUILD_DIR"
