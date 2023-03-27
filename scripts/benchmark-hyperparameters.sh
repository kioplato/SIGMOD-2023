#!/bin/bash

# Benchmark the clustering solution.
# We can specify the hyperparameters to benchmark.
# The benchmarks results are outputed as markdown table.

# The absolute path to this script.
_SCRIPT_ABSOLUTE_PATH=$(readlink -f "$0")
# This script's directory (this script lives in the scripts/ directory).
_SCRIPT_DIRECTORY=$(dirname "$_SCRIPT_ABSOLUTE_PATH")
# Project's root directory.
_ROOT_DIR=$(readlink -f "$_SCRIPT_DIRECTORY/..")

unset _SCRIPT_ABSOLUTE_PATH
unset _SCRIPT_DIRECTORY

source "$_ROOT_DIR/scripts/die.sh"
source "$_ROOT_DIR/scripts/timer.sh"
source "$_ROOT_DIR/scripts/logfile.sh"
source "$_ROOT_DIR/scripts/find-unique-filename.sh"

# Verbose default value is 0.
if [ -z "$VERBOSE" ]
then
	VERBOSE=0
fi

arguments=("$@")
for ((i=0; i<${#arguments[@]}; ++i))
do
	case ${arguments[$i]} in
		"--verbose")
			VERBOSE=1
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
		"--cutoff")
			((++i))
			CUTOFF=${arguments[$i]}
			;;
		*)
			die "flag ${arguments[$i]} not recognized."
	esac
done

unset arguments

# Dataset path must be set.
if [ -z "$DATASET_PATH" ]
then
	die "--dataset <path> not provided."
else
	# The dataset path must exist.
	if [ ! -e "$DATASET_PATH" ]
	then
		die "specified dataset \"$DATASET_PATH\" does not exist."
	# The dataset path must be a regular file.
	elif [ ! -f "$DATASET_PATH" ]
	then
		die "specified dataset \"$DATASET_PATH\" isn't a regular file."
	fi
fi

# Ground truth path must be set.
if [ -z "$GROUNDTRUTH_PATH" ]
then
	die "--ground-truth <path> not provided."
else
	# The ground truth path must exist.
	if [ ! -e "$GROUNDTRUTH_PATH" ]
	then
		die "specified ground truth \"$GROUNDTRUTH_PATH\" does not exist."
	# The ground truth path must be a regular file.
	elif [ ! -f "$GROUNDTRUTH_PATH" ]
	then
		die "specified ground truth \"$GROUNDTRUTH_PATH\" isn't a regular file."
	fi
fi

# Minimum number of clusters must be set.
if [ -z "$NUM_CLUSTERS_MIN" ]
then
	die "--num-clusters-min is not provided."
# Maximum number of clusters must be set.
elif [ -z "$NUM_CLUSTERS_MAX" ]
then
	die "--num-clusters-max is not provided."
# Number of clusters increase step must be set.
elif [ -z "$NUM_CLUSTERS_STEP" ]
then
	die "--num-clusters-step is not provided."
# Min number of clusters must be less or equal to max number of clusters.
elif [ "$NUM_CLUSTERS_MIN" -gt "$NUM_CLUSTERS_MAX" ]
then
	die "--num-clusters-min must be less or equal to --num-clusters-max."
# Step size must be positive.
elif [ "$NUM_CLUSTERS_STEP" -le 0 ]
then
	die "--num-clusters-step must be positive."
fi

# Minimum number of iterations must be set.
if [ -z "$NUM_ITERS_MIN" ]
then
	die "--n-iters-min is not provided."
# Maximum number of clusters must be set.
elif [ -z "$NUM_ITERS_MAX" ]
then
	die "--n-iters-max is not provided."
# Number of clusters increase step must be set.
elif [ -z "$NUM_ITERS_STEP" ]
then
	die "--num-iters-step is not provided."
# Min number of clusters must be less or equal to max number of clusters.
elif [ "$NUM_ITERS_MIN" -gt "$NUM_ITERS_MAX" ]
then
	die "--num-iters-min must be less or equal to --num-iters-max."
# Step size must be positive.
elif [ "$NUM_ITERS_STEP" -le 0 ]
then
	die "--num-iters-step must be positive."
fi

# Minimum neighborhood size must be set.
if [ -z "$NUM_NEAREST_CLUSTERS_MIN" ]
then
	die "--n-nearest-clusters-min is not provided."
# Maximum neighborhood size must be set.
elif [ -z "$NUM_NEAREST_CLUSTERS_MAX" ]
then
	die "--n-nearest-clusters-max is not provided."
# Neighborhood size increase step must be set.
elif [ -z "$NUM_NEAREST_CLUSTERS_STEP" ]
then
	die "--n-nearest-clusters-step is not provided."
# Min neighborhood size must be less or equal to max neighborhood size.
elif [ "$NUM_NEAREST_CLUSTERS_MIN" -gt "$NUM_NEAREST_CLUSTERS_MAX" ]
then
	die "--n-nearest-clusters-min must be less or equal to --n-nearest-clusters-max."
# Step size must be positive.
elif [ "$NUM_NEAREST_CLUSTERS_STEP" -le 0 ]
then
	die "--n-nearest-clusters-step must be positive."
fi

# Cutoff must be set.
if [ -z "$CUTOFF" ]
then
	die "--cutoff <seconds> is not provided."
elif ! [[ "$CUTOFF" =~ [1-9]+[0-9]* ]]
then
	die "--cutoff <seconds> is not a positive number."
fi

# @unique_file describes the benchmarks performed in the filename.
unique_file="hyperparam-benchmark-clusters"
unique_file+="-dataset-$(basename -s .bin "$DATASET_PATH")"
unique_file+="-clusters-$NUM_CLUSTERS_MIN-$NUM_CLUSTERS_STEP-$NUM_CLUSTERS_MAX"
unique_file+="-iters-$NUM_ITERS_MIN-$NUM_ITERS_STEP-$NUM_ITERS_MAX"
unique_file+="-n-nearest-clusters-$NUM_NEAREST_CLUSTERS_MIN-$NUM_NEAREST_CLUSTERS_STEP-$NUM_NEAREST_CLUSTERS_MAX"

# Find unique file name for the log file and output file.
find_unique_filename unique_file

# Set the log file path.
set-log-file-path "$unique_file.log"
# Set the output file path.
_OUTPUT_FILE="$unique_file.txt"

unset unique_file

# Where to build the project.
_BUILD_DIR=$(mktemp -d /tmp/SIGMOD-BENCHMARKS-Build.XXXXXXXXXX)
# The number of cores to use for compilation.
_N_PROCS=$(nproc)

log "Benchmarks started."
log "Command used: $0 $*"
log "Log file: $__LOG_FILE"
log "Benchmark's file: $_OUTPUT_FILE"
log "Project's root directory: $_ROOT_DIR"
log "Project's build directory: $_BUILD_DIR"
log "Using $_N_PROCS threads for compilation."

# Compile the project in subshell.
(
	# Go into the project's root directory.
	cd "$_ROOT_DIR" || diei "could not cd $_ROOT_DIR"
	# Generate makefile.
	cmake -B"$_BUILD_DIR" -DVERBOSE=$VERBOSE >/dev/null 2>&1 || diei "failed to cmake -BBuild"
	# cd into Build/ directory.
	cd "$_BUILD_DIR" || diei "could not cd $_BUILD_DIR"
	# Compile the project.
	make -j"$_N_PROCS" >/dev/null 2>&1 || diei "could not make -j8"
)

log "Compiled project."

# Find the executable's name from CMakeLists.txt file.
_EXE_BASENAME=$(grep -o "project([a-zA-Z0-9+]*)" "$_ROOT_DIR/CMakeLists.txt" | grep -oP "project\(\K.*?(?=\))")
# The executable's path.
_EXE_PATH="$_BUILD_DIR/$_EXE_BASENAME"

unset _EXE_BASENAME

log "The executable's path: $_EXE_PATH"

# The path to the evaluator executable.
_RECALLER_PATH="$_BUILD_DIR/recaller"

# Compile the evaluation program.
(
	# Go into the project's root directory.
	cd "$_ROOT_DIR" || diei "couldn't cd into $_ROOT_DIR"
	# Compile the evaluation program.
	g++ -o "$_RECALLER_PATH" evaluator/recall.cpp -fopenmp
)

log "Compiled recaller."
log "The recaller's path: $_RECALLER_PATH"

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
	echo "<!-- Number of nearest clusters in [$NUM_NEAREST_CLUSTERS_MIN:$NUM_NEAREST_CLUSTERS_STEP:$NUM_NEAREST_CLUSTERS_MAX]. -->"
	echo
	echo "<!-- Some benchmarks may fail because the algorithm cound not find 100 nearest neighbor for at least one point. -->"
	echo "<!-- In this case the Success column will be False and the next benchmark will skip the rest # Nearest clusters -->"
	echo "<!-- values and proceed with the next # Clusters value. That's because decreasing the # Nearest clusters we search -->"
	echo "<!-- into will also decrease the nearest neighbors we can find. This means that the next benchmark will also fail. -->"
	echo "<!-- That's the reason we iterate the # Nearest clusters in reverse, to be able to skip unsuccessful benchmarks. -->"
	echo
	echo "| Dataset | # Clusters | # Nearest clusters | # Iterations | Elapsed time | Recall | Exit status |"
	echo "|---------|------------|--------------------|--------------|--------------|--------|-------------|"
} > "$_OUTPUT_FILE"

dataset_basename=$(basename "${DATASET_PATH%.*}")

# We would like to store the benchmarks in three different ways.
# First way is by hyperparameter significance: clusters, nearest clusters, iters.
# Second is by execution time in increasing order.
# Third is by decreasing recall score and then by increasing execution time.
#
# To achieve the above we use `sort` utility. To make our lifes easier we use
# a seperate file to store the markdown table and sort it there, without the
# leading comments and table's column names.
results_tmp=$(mktemp /tmp/hyperparam-benchmark-results.XXXXXXXXXX)

# We need a file to write each benchmark's aprox knng.
aprox_knng_tmp=$(mktemp /tmp/benchmark-knng-output.bin.XXXXXXXXXX)

# A file to write the `time`'s calculated execution time of each benchmark.
elapsed_time_tmp=$(mktemp /tmp/elapsed_time.XXXXXXXXXX)

# Because of the backgrounded benchmark we can't get the solution's output.
# We need a temporary file to redirect its output there.
benchmark_stdout_stderr=$(mktemp /tmp/benchmark_stdout_stderr.XXXXXXXXXX)

log "Starting iterating over hyperparameter combinations."

# Kill the slow hyperparameter combination.
function timer-callback
{
	pid_to_kill=$1

	kill "$pid_to_kill"
}

# Iterate over all the hyperparameter combinations and benchmark.
# There's no point in creating forks. The solution creates threads.
for ((n_clusters=NUM_CLUSTERS_MIN; n_clusters <= NUM_CLUSTERS_MAX; n_clusters += NUM_CLUSTERS_STEP))
do
	# If we have 5 clusters and NUM_NEAREST_CLUSTERS_MAX is 20, then we start from 5.
	# If we have 20 clusters and NUM_NEAREST_CLUSTERS_MAX is 5, then we start from 5.
	n_nearest_clusters=$((n_clusters < NUM_NEAREST_CLUSTERS_MAX ? n_clusters : NUM_NEAREST_CLUSTERS_MAX))

	for ((; n_nearest_clusters >= NUM_NEAREST_CLUSTERS_MIN; n_nearest_clusters -= NUM_NEAREST_CLUSTERS_STEP))
	do
		for ((n_iters=NUM_ITERS_MIN; n_iters <= NUM_ITERS_MAX; n_iters += NUM_ITERS_STEP))
		do
			log "===== Benchmarking with $n_clusters clusters, $n_nearest_clusters neighborhood size and $n_iters iterations. ====="
			log "Standard output and error of the program follows."
			log "BENCHMARK START"

			# `mktemp` creates the file.
			rm -f "$aprox_knng_tmp"

			# Run the benchmark with the current hyperparameters.
			command time --format "%e" --quiet --output="$elapsed_time_tmp"\
				"$_EXE_PATH" --dataset "$DATASET_PATH"\
				--n-clusters $n_clusters --n-iters $n_iters\
				--n-nearest-clusters $n_nearest_clusters\
				--output "$aprox_knng_tmp" >"$benchmark_stdout_stderr" 2>&1 &

			# The pid of the benchmark. We need to wait on it.
			benchmarks_pid=$!

			# Set a timer to cutoff the benchmark if its elapsed
			# time surpasses the cutoff value set. We don't want
			# to wait on bad hyperparameter combinations.
			start-timer "$CUTOFF" "timer-callback $benchmarks_pid"

			# Wait on the benchmark.
			wait $benchmarks_pid
			benchmark_exit_code=$?

			stop-timer

			# Write the output to the log file.
			# @exe_output will have contents if --verbose is set.
			readarray -t exe_output < "$benchmark_stdout_stderr"
			for line in "${exe_output[@]}"
			do
				log "$line"
			done

			log "Benchmark's return value = $benchmark_exit_code"

			log "BENCHMARK END"
			log "==========================================================="

			# We need to check each case: successful execution,
			# terminated by SIGUSR1 and internal error by bug.

			# Successful execution.
			if [ $benchmark_exit_code -eq 0 ]
			then
				elapsed_time=$(cat "$elapsed_time_tmp")

				# Evaluate the recall of the output.bin file.
				eval_output=$($_RECALLER_PATH --true-knng-path "$GROUNDTRUTH_PATH" --eval-knng-path "$aprox_knng_tmp")

				# Get the recall score from @eval_output.
				recall=$(echo "$eval_output" | grep -o "Recall score:[[:blank:]][0-9.]*" | awk '{print $3}')

				benchmark_success="Completed"
			# The benchmark exceeded @CUTOFF time.
			elif [ $benchmark_exit_code -eq 138 ]
			then
				elapsed_time=">$CUTOFF"
				recall="NaN"

				# Since this benchmarked exceeded the specified CUTOFF value, the next benchmark with more K-Means
				# iterations will require even more time. Therefore we continue with the next nearest clusters value.
				n_iters=$((NUM_ITERS_MAX + 1))

				benchmark_success="Cutoff exceeded"
			# The clusters are too many and the nearest clusters value too small to find 100 nearest neighbors.
			else
				elapsed_time="NaN"
				recall="NaN"

				benchmark_success="Can't find 100 nn"
			fi

			# Write this benchmark's results.
			echo "| $dataset_basename | $n_clusters | $n_nearest_clusters | $n_iters | $elapsed_time | $recall | $benchmark_success |" >> "$results_tmp"

			rm -f "$elapsed_time_tmp"

			if [ $benchmark_exit_code -ne 0 ] && [ $benchmark_exit_code -ne 138 ]
			then
				# Proceed with the next @c_clusters value.
				n_iters=$((NUM_ITERS_MAX + 1))
				n_nearest_clusters=0
			fi
		done
	done
done

rm -f "$aprox_knng_tmp"

# Append the hyperparameter benchmark results in the three sorted orders.
{
	# Sort by hyperparameter significance.
	cat "$results_tmp"

	# Sort by execution time in increasing order.
	echo
	echo "| Dataset | # Clusters | # Nearest clusters | # Iterations | Elapsed time | Recall | Success |"
	echo "|---------|------------|--------------------|--------------|--------------|--------|---------|"
	sort -k10 -n "$results_tmp"

	# Sort by recall score and then by increasing execution time.
	echo
	echo "| Dataset | # Clusters | # Nearest clusters | # Iterations | Elapsed time | Recall | Success |"
	echo "|---------|------------|--------------------|--------------|--------------|--------|---------|"
	sort -k12r -k10 -n "$results_tmp"
} >> "$_OUTPUT_FILE"

log "Hyperparameter benchmarks complete."

log "Cleaning up /tmp directory."
rm -rf "$_BUILD_DIR"
rm -rf "$results_tmp"
rm -f "$benchmark_stdout_stderr"

log "Results written at $_OUTPUT_FILE"
