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
	// Don't use dynamic number of threads.
	omp_set_dynamic(0);
	// Don't use nested parallelism.
	omp_set_nested(0);
	// The number of threads we want matches the number of cores we have.
	//omp_set_num_threads(min(omp_get_num_procs(), omp_get_max_threads()));
	omp_set_num_threads(omp_get_num_procs());
	// TODO: Set OMP_PROC_BIND env var.

	// The default path of the dataset.
	string dataset_path = "datasets/dummy-data.bin";

	// Also accept other path for source data.
	if (argc > 1) dataset_path = string(argv[1]);

	cout << "Dataset path = " << dataset_path << endl;

	// Read dataset points.
	vector<point_t> points = read_dataset(dataset_path, 100);
	//cout << "Read " << points.size() << " points." << endl;

	const uint32_t k = 100;

	// Construct the knng.
	vector<vector<uint32_t>> knng = create_knng(points, k);

	// Save to ouput.bin file.
	write_knng(knng, 100, "output.bin");

	return 0;
}
