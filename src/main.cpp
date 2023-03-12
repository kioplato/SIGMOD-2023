#include <iostream>
#include <string>
#include <vector>
#include "knng.hpp"
#include "point.hpp"
#include "helpers.hpp"
#include "input-output.hpp"

using namespace std;

int main(int argc, char **argv)
{
	// The default path of the dataset.
	string dataset_path = "dummy-data.bin";

	// Also accept other path for source data.
	if (argc > 1) dataset_path = string(argv[1]);

	// Read dataset points.
	vector<point_t> points = read_dataset(dataset_path, 100);

	points_print(points, "/dev/stdout");

	return 0;

	// Construct the knng.
	vector<vector<uint32_t>> knng = create_knng(points, 100);

	// Save to ouput.bin file.
	write_knng(knng, 100, "output.bin");

	return 0;
}
