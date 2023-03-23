#!/bin/bash

# Purpose: download a specific dataset of the contest.
# Three datasets exist with cardinalities: 10k, 1m and 10m.

_SCRIPT_BASENAME=$(basename "$0")

function die()
{
	# String to print to stdout.
	str=$1

	echo "[$_SCRIPT_BASENAME] fatal: $str"
	echo "run $_SCRIPT_BASENAME --help for usage."

	exit 1
}

arguments=("$@")
for ((c_arg=0; c_arg < ${#arguments[@]}; ++c_arg))
do
	case ${arguments[$c_arg]} in
		"--help")
			usage
			exit
			;;
		"--dataset-cardinality")
			((++c_arg))
			DATASET_CARDINALITY=${arguments[$c_arg]}
			;;
		"--download-directory")
			((++c_arg))
			DOWNLOAD_DIR=${arguments[$c_arg]}
			;;
		*)
			die "flag ${arguments[$c_arg]} not recognized."
	esac
done

if [ -z "$DATASET_CARDINALITY" ]
then
	die "--dataset-cardinality not provided."
elif [ -z "$DOWNLOAD_DIR" ]
then
	die "--download-directory not provided."
fi

if [ "$DATASET_CARDINALITY" == "10k" ]
then
	curl https://sigmod23storage.blob.core.windows.net/sigmod2023contest/dummy-data.bin -o "$DOWNLOAD_DIR/10k.bin"
elif [ "$DATASET_CARDINALITY" == "1m" ]
then
	curl https://sigmod23storage.blob.core.windows.net/sigmod2023contest/contest-data-release-1m.bin -o "$DOWNLOAD_DIR/1m.bin"
elif [ "$DATASET_CARDINALITY" == "10m" ]
then
	curl https://sigmod23storage.blob.core.windows.net/sigmod2023contest/contest-data-release-10m.bin -o "$DOWNLOAD_DIR/10m.bin"
elif [ "$DATASET_CARDINALITY" == "all" ]
then
	curl https://sigmod23storage.blob.core.windows.net/sigmod2023contest/dummy-data.bin -o "$DOWNLOAD_DIR/10k.bin" &
	curl https://sigmod23storage.blob.core.windows.net/sigmod2023contest/contest-data-release-1m.bin -o "$DOWNLOAD_DIR/1m.bin" &
	curl https://sigmod23storage.blob.core.windows.net/sigmod2023contest/contest-data-release-10m.bin -o "$DOWNLOAD_DIR/10m.bin" &

	wait
else
	die "--dataset-cardinality can only be 10k, 1m or 10m."
fi
