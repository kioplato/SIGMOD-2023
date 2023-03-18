#include <iostream>
#include <string>
#include <vector>
#include <omp.h>
#include "knng.hpp"
#include "point.hpp"
#include "helpers.hpp"
#include "input-output.hpp"

using namespace std;

int main(int argc, char **argv)
{
#ifdef VERBOSE
	const clock_t time_start = clock();
#endif

	// TODO: Set OMP_PROC_BIND env var.

	// Don't use dynamic number of threads.
	omp_set_dynamic(0);
	// Don't use nested parallelism.
	omp_set_nested(0);
	// The number of threads we want matches the number of cores we have.
	//omp_set_num_threads(min(omp_get_num_procs(), omp_get_max_threads()));
	omp_set_num_threads(omp_get_num_procs());

#ifdef VERBOSE
	cout << "Parsing command-line arguments." << endl;
#endif

	// Parse @argv, verify its correctness and store the flags in @config.
	config_t config = parse_argv(argc, argv);

#ifdef VERBOSE
	print_config(config);
#endif

	/*
	 * The number of neighbors to find per point.
	 * This is a fixed hyperparameter of the problem.
	 */
	const uint32_t k = 100;

	/*
	 * The number of dimensions each point has.
	 * This is a fixed hyperparameter of the problem.
	 */
	const uint32_t n_dims = 100;

#ifdef VERBOSE
	cout << "Reading " << config.dataset << " dataset." << endl;
#endif

	// Read dataset points.
	points_t points = read_dataset(config.dataset, n_dims);

#ifdef VERBOSE
	cout << "Read " << points.size() << " points." << endl;
#endif

	// Construct the knng.
	knng_t knng = create_knng(points, k, config.n_clusters, config.n_iters);

#ifdef VERBOSE
	cout << "Writing the knng to " << config.output << " file." << endl;
#endif

	// Save knng to the specified file through commane-line arguments.
	write_knng(knng, k, config.output);

#ifdef VERBOSE
	const clock_t time_stop = clock();

	/*
	 * Calculated time is the elapsed time of all the threads.
	 * Therefore we need to divide by the number of used threads.
	 */
	const double duration = (1000.0 * (time_stop - time_start) / CLOCKS_PER_SEC) / omp_get_num_procs();

	cout << "Elapsed time = " << duration << endl;
#endif

	return EXIT_SUCCESS;
}
