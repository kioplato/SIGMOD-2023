#!/bin/bash

# Benchmark the clustering solution.
# We can specify the hyperparameters to benchmark.
# The benchmarks results are outputed as markdown table.

# The file name of the script.
SCRIPT_BASENAME=$(basename "$0")

# Print provided string and die.
function die()
{
	# String to print to stdout.
	str=$1

	echo "[$SCRIPT_BASENAME] fatal: $str"
	echo "run $SCRIPT_BASENAME --help for usage."

	exit 1
}

function diei()
{
	# String to print to stdout.
	str=$1

	echo "[$SCRIPT_BASENAME] internal: $str"
	echo "run $SCRIPT_BASENAME --help for usage."

	exit 1
}

# Print provided string to the log file.
function log()
{
	# The string to write to the log file.
	str=$1

	current_time=$(date +"[%d/%m/%y %H:%M:%S]")

	# Write the current time and the provided string to the log file.
	echo "$current_time: $str" >> "$LOG_FILE"
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
			# The path to the dataset to use for the benchmarks.
			DATASET_PATH=${arguments[$i]}
			;;
		"--ground-truth")
			((++i))
			# The ground truth file of the dataset.
			GROUNDTRUTH_PATH=${arguments[$i]}
			;;
		"--n-clusters-min")
			((++i))
			# The lower bound number of clusters to benchmark.
			NUM_CLUSTERS_MIN=${arguments[$i]}
			;;
		"--n-clusters-max")
			((++i))
			# The upper bound number of clusters to benchmark.
			NUM_CLUSTERS_MAX=${arguments[$i]}
			;;
		"--n-clusters-step")
			((++i))
			# The increasing step of number of clusters to benchmark.
			NUM_CLUSTERS_STEP=${arguments[$i]}
			;;
		"--n-iters-min")
			((++i))
			# The lower bound of iterations to perform.
			NUM_ITERS_MIN=${arguments[$i]}
			;;
		"--n-iters-max")
			((++i))
			# The upper bound of iterations to perform.
			NUM_ITERS_MAX=${arguments[$i]}
			;;
		"--n-iters-step")
			((++i))
			# The increasing step of number of iterations to perform.
			NUM_ITERS_STEP=${arguments[$i]}
			;;
		"--n-nearest-clusters-min")
			((++i))
			# The lower bound of neighboring clusters to search.
			NUM_NEAREST_CLUSTERS_MIN=${arguments[$i]}
			;;
		"--n-nearest-clusters-max")
			((++i))
			# The upper bound of neighboring clusters to search.
			NUM_NEAREST_CLUSTERS_MAX=${arguments[$i]}
			;;
		"--n-nearest-clusters-step")
			((++i))
			# The step size of neighboring clusters to search.
			NUM_NEAREST_CLUSTERS_STEP=${arguments[$i]}
			;;
		*)
			die "fatal: flag ${arguments[$i]} not recognized."
	esac
done

# Dataset path must be set.
if [ -z "$DATASET_PATH" ]
then
	die "fatal: --dataset <path> not provided."
else
	# The dataset path must exist.
	if [ ! -e "$DATASET_PATH" ]
	then
		die "fatal: specified dataset \"$DATASET_PATH\" does not exist."
	# The dataset path must be a regular file.
	elif [ ! -f "$DATASET_PATH" ]
	then
		die "fatal: specified dataset \"$DATASET_PATH\" isn't a regular file."
	fi
fi

# Ground truth path must be set.
if [ -z "$GROUNDTRUTH_PATH" ]
then
	die "fatal: --ground-truth <path> not provided."
else
	# The ground truth path must exist.
	if [ ! -e "$GROUNDTRUTH_PATH" ]
	then
		die "fatal: specified ground truth \"$GROUNDTRUTH_PATH\" does not exist."
	# The ground truth path must be a regular file.
	elif [ ! -f "$GROUNDTRUTH_PATH" ]
	then
		die "fatal: specified ground truth \"$GROUNDTRUTH_PATH\" isn't a regular file."
	fi
fi

# Minimum number of clusters must be set.
if [ -z "$NUM_CLUSTERS_MIN" ]
then
	die "fatal: --num-clusters-min is not provided."
# Maximum number of clusters must be set.
elif [ -z "$NUM_CLUSTERS_MAX" ]
then
	die "fatal: --num-clusters-max is not provided."
# Number of clusters increase step must be set.
elif [ -z "$NUM_CLUSTERS_STEP" ]
then
	die "fatal: --num-clusters-step is not provided."
# Min number of clusters must be less or equal to max number of clusters.
elif [ "$NUM_CLUSTERS_MIN" -gt "$NUM_CLUSTERS_MAX" ]
then
	die "fatal: --num-clusters-min must be less or equal to --num-clusters-max."
# Step size must be positive.
elif [ "$NUM_CLUSTERS_STEP" -le 0 ]
then
	die "fatal: --num-clusters-step must be positive."
fi

# Minimum number of iterations must be set.
if [ -z "$NUM_ITERS_MIN" ]
then
	die "fatal: --n-iters-min is not provided."
# Maximum number of clusters must be set.
elif [ -z "$NUM_ITERS_MAX" ]
then
	die "fatal: --n-iters-max is not provided."
# Number of clusters increase step must be set.
elif [ -z "$NUM_ITERS_STEP" ]
then
	die "fatal: --num-iters-step is not provided."
# Min number of clusters must be less or equal to max number of clusters.
elif [ "$NUM_ITERS_MIN" -gt "$NUM_ITERS_MAX" ]
then
	die "fatal: --num-iters-min must be less or equal to --num-iters-max."
# Step size must be positive.
elif [ "$NUM_ITERS_STEP" -le 0 ]
then
	die "fatal: --num-iters-step must be positive."
fi

# Minimum neighborhood size must be set.
if [ -z "$NUM_NEAREST_CLUSTERS_MIN" ]
then
	die "fatal: --n-nearest-clusters-min is not provided."
# Maximum neighborhood size must be set.
elif [ -z "$NUM_NEAREST_CLUSTERS_MAX" ]
then
	die "fatal: --n-nearest-clusters-max is not provided."
# Neighborhood size increase step must be set.
elif [ -z "$NUM_NEAREST_CLUSTERS_STEP" ]
then
	die "fatal: --n-nearest-clusters-step is not provided."
# Min neighborhood size must be less or equal to max neighborhood size.
elif [ "$NUM_NEAREST_CLUSTERS_MIN" -gt "$NUM_NEAREST_CLUSTERS_MAX" ]
then
	die "fatal: --n-nearest-clusters-min must be less or equal to --n-nearest-clusters-max."
# Step size must be positive.
elif [ "$NUM_NEAREST_CLUSTERS_STEP" -le 0 ]
then
	die "fatal: --n-nearest-clusters-step must be positive."
fi

# Find unique file name for the log file and output file.
benchmark_id="dataset-$(basename -s .bin "$DATASET_PATH")-clusters-$NUM_CLUSTERS_MIN-$NUM_CLUSTERS_STEP-$NUM_CLUSTERS_MAX-iters-$NUM_ITERS_MIN-$NUM_ITERS_STEP-$NUM_ITERS_MAX-n-nearest-clusters-$NUM_NEAREST_CLUSTERS_MIN-$NUM_NEAREST_CLUSTERS_STEP-$NUM_NEAREST_CLUSTERS_MAX"
# The files' prefix that describe the benchmark.
file_prefix="hyperparam-benchmark-clusters-$benchmark_id"
# A number that distinguishes this benchmark's files with similar benchmarks.
file_version=1
# File extensions.
output_file_suffix="txt"
log_file_suffix="log"
# The actual output file path.
OUTPUT_FILE="$file_prefix-$file_version.$output_file_suffix"
# The actual log file path.
LOG_FILE="$file_prefix-$file_version.$log_file_suffix"

# Start increasing @file_version until we find a file that does not exist.
while [ -f "$OUTPUT_FILE" ] || [ -f "$LOG_FILE" ]
do
	((++file_version))
	OUTPUT_FILE="$file_prefix-$file_version.$output_file_suffix"
	LOG_FILE="$file_prefix-$file_version.$log_file_suffix"
done

log "Benchmarks started."
log "Command used: $0 $*"

# The absolute path to this script.
SCRIPT_ABSOLUTE_PATH=$(readlink -f "$0")
# This script's directory (this script lives in the scripts/ directory).
SCRIPT_DIRECTORY=$(dirname "$SCRIPT_ABSOLUTE_PATH")
# Project's root directory.
ROOT_DIR=$(readlink -f "$SCRIPT_DIRECTORY/..")

log "Project's root directory: $ROOT_DIR"

BUILD_DIR="/tmp/SIGMOD-Build"

log "Project's build directory: $BUILD_DIR"

# The number of cores to use for compilation.
N_PROCS=$(nproc)

log "Using $N_PROCS threads for compilation."

# Compile the project in subshell.
(
	# Go into the project's root directory.
	cd "$ROOT_DIR" || die "internal: could not cd $ROOT_DIR"
	# Generate makefile.
	cmake -B"$BUILD_DIR" >/dev/null 2>&1 || die "internal: failed to cmake -BBuild"
	# cd into Build/ directory.
	cd "$BUILD_DIR" || die "internal: could not cd $BUILD_DIR"
	# Compile the project.
	make -j"$N_PROCS" >/dev/null 2>&1 || die "internal: could not make -j8"
)

log "Compiled project."

# Find the executable's name from CMakeLists.txt file.
EXE_BASENAME=$(grep -o "project([a-zA-Z0-9+]*)" "$ROOT_DIR/CMakeLists.txt" |
	grep -oP "project\(\K.*?(?=\))")
# The executable's path.
EXE_PATH="$BUILD_DIR/$EXE_BASENAME"
log "The executable's path: $EXE_PATH"

# The path to the evaluator executable.
EVAL_PATH="$BUILD_DIR/evaluate"

# Compile the evaluation program.
(
	# Go into the project's root directory.
	cd "$ROOT_DIR" || die "internal: couldn't cd into $ROOT_DIR"
	# Compile the evaluation program.
	g++ -o "$EVAL_PATH" evaluator/recall.cpp -fopenmp
)

log "Compiled evaluator."
log "The evaluator's path: $EVAL_PATH"

log "Starting iterating over hyperparameter combinations."
KNNG_OUTPUT="/tmp/benchmark-knng-output.bin"

{
	echo "<!-- Benchmarks ran using the scripts/$(basename "$0") script. -->"
	echo
	echo "<!-- Command used to run the script: -->"
	echo "<!-- $0 $* -->"
	echo
	echo "<!-- Dataset benchmarked: -->"
	echo "<!-- $DATASET_PATH -->"
	echo
	echo "<!-- Hyperparameters tested: -->"
	echo "<!-- Number of clusters in [$NUM_CLUSTERS_MIN:$NUM_CLUSTERS_STEP:$NUM_CLUSTERS_MAX]. -->"
	echo "<!-- K-Means iterations in [$NUM_ITERS_MIN:$NUM_ITERS_STEP:$NUM_ITERS_MAX]. -->"
	echo "<!-- Neighborhood search depth in [$NUM_NEAREST_CLUSTERS_MIN:$NUM_NEAREST_CLUSTERS_STEP:$NUM_NEAREST_CLUSTERS_MAX]. -->"
	echo
	echo "| Dataset | # Clusters | # Nearest clusters | # Iterations | Elapsed time | Recall |"
	echo "|---------|------------|--------------------|--------------|--------------|--------|"
} > "$OUTPUT_FILE"

DATASET_BASENAME=$(basename "${DATASET_PATH%.*}")

# We would like to store the benchmarks in three different ways.
# First way is by hyperparameter significance: clusters, nearest clusters, iters.
# Second is by execution time in increasing order.
# Third is by decreasing recall score and then by increasing execution time.
#
# To achieve the above we use `sort` utility. To make our lifes easier we use
# a seperate file to store the markdown table and sort it there, without the
# leading comments and table's column names.
results_tmp=$(mktemp /tmp/hyperparam-benchmark-results.XXXXXXXXXX)

# Iterate over all the hyperparameter combinations and benchmark.
# There's no point in creating forks. The solution creates threads.
for ((n_clusters=NUM_CLUSTERS_MIN; n_clusters <= NUM_CLUSTERS_MAX; n_clusters += NUM_CLUSTERS_STEP))
do
	for ((n_nearest_clusters=NUM_NEAREST_CLUSTERS_MIN; n_nearest_clusters <= n_clusters; n_nearest_clusters += NUM_NEAREST_CLUSTERS_STEP))
	do
		for ((n_iters=NUM_ITERS_MIN; n_iters <= NUM_ITERS_MAX; n_iters += NUM_ITERS_STEP))
		do
			log "===== Benchmarking with $n_clusters clusters, $n_nearest_clusters neighborhood size and $n_iters iterations. ====="
			log "Standard output and error of the program follows."
			log "BENCHMARK START"

			# Run the benchmark with the current hyperparameters.
			exe_output="$(command time --format "%e" --quiet --output=/tmp/elapsed-time.txt "$EXE_PATH" --dataset "$DATASET_PATH" --n-clusters $n_clusters --n-iters $n_iters --n-nearest-clusters $n_nearest_clusters --output "$KNNG_OUTPUT" 2>&1)"
			exe_success=$?

			# Write the output to the log file.
			readarray -t exe_output <<< "$exe_output"
			for line in "${exe_output[@]}"
			do
				log "$line"
			done

			log "BENCHMARK END"
			log "==========================================================="

			# The benchmark could fail.
			if [ $exe_success -ne 0 ]
			then
				echo "| $DATASET_BASENAME | $n_clusters | $n_nearest_clusters | $n_iters | Internal error | Internal error |" >> "$results_tmp"

				continue
			fi

			elapsed_time=$(cat /tmp/elapsed-time.txt)
			rm -f "/tmp/elapsed-time.txt"

			# Evaluate the recall of the output.bin file.
			eval_output=$($EVAL_PATH --true-knng-path "$GROUNDTRUTH_PATH" --eval-knng-path "$KNNG_OUTPUT")
			# Get the recall score from @eval_output.
			recall=$(echo "$eval_output" |
				grep -o "Recall score:[[:blank:]][0-9.]*" |
				awk '{print $3}'
			)

			# Write this benchmark's results.
			echo "| $DATASET_BASENAME | $n_clusters | $n_nearest_clusters | $n_iters | $elapsed_time | $recall |" >> "$results_tmp"

			rm -f "$KNNG_OUTPUT"
		done
	done
done

# Append the hyperparameter benchmark results in the three sorted orders.
{
	# Sort by hyperparameter significance.
	cat "$results_tmp"

	# Sort by execution time in increasing order.
	echo
	echo "| Dataset | # Clusters | # Nearest clusters | # Iterations | Elapsed time | Recall |"
	echo "|---------|------------|--------------------|--------------|--------------|--------|"
	sort -k10 -n "$results_tmp"

	# Sort by recall score and then by increasing execution time.
	echo
	echo "| Dataset | # Clusters | # Nearest clusters | # Iterations | Elapsed time | Recall |"
	echo "|---------|------------|--------------------|--------------|--------------|--------|"
	sort -k12r -k10 -n "$results_tmp"
} >> "$OUTPUT_FILE"

log "Hyperparameter benchmarks complete."

log "Cleaning up /tmp directory."
rm -rf $BUILD_DIR
rm -rf "$results_tmp"

log "Results written at $OUTPUT_FILE"
