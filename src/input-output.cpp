#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include "input-output.hpp"
#include "helpers.hpp"

using namespace std;

static inline void
valid_dataset_or_die(string& config_dataset, string dataset)
{
	if (!config_dataset.empty())
		die("fatal: --dataset <path> already provided.");

	// Check the existance of the dataset.
	struct stat buf;
	if (stat(dataset.c_str(), &buf))
		die("fatal: dataset path does not exist.");

	config_dataset = dataset;
}

static inline void
valid_n_clusters_or_die(uint32_t& config_n_clusters, string n_clusters)
{
	if (config_n_clusters != 0)
		die("fatal: --n-clusters <unsigned> already provided.");

	// Check that the provided @n_clusters is positive.
	if (!regex_match(n_clusters, regex("[1-9]+[0-9]*")))
		die("fatal: --n-clusters ", n_clusters, " is not a positive number.");

	config_n_clusters = atoll(n_clusters.c_str());
}

static inline void
valid_n_iters_or_die(uint32_t& config_n_iters, string n_iters)
{
	if (config_n_iters != 0)
		die("fatal: --n-iters <unsigned> already provided.");

	// Check that the provided @n_iters is positive.
	if (!regex_match(n_iters, regex("[1-9]+[0-9]*")))
		die("fatal: --n-iters ", n_iters, " is not a positive number.");

	config_n_iters = atoll(n_iters.c_str());
}

static inline void
valid_output_or_die(string& config_output, string output)
{
	if (!config_output.empty())
		die("fatal: --output <path> already provided.");

	// Check the existance of the output file.
	struct stat buf;
	if (!stat(output.c_str(), &buf))
		die("fatal: output path already exists.");

	config_output = output;
}

config_t parse_argv(uint32_t argc, char **argv)
{
	/*
	 * We require four command-line arguments.
	 * 1. Program's name.
	 * 2. Dataset path.
	 * 3. Number of clusters.
	 * 4. Number of iterations (for K-Means).
	 */

	config_t config;

	// Read the command-line arguments.
	for (uint32_t index = 1; index < argc; ++index) {
		string key(argv[index]);

		// There's no token following current flag.
		if (index == argc - 1)
			die("fatal: flag ", argv[index], " isn't followed by a value.");

		string val(argv[++index]);

		if (key == "--dataset")
			valid_dataset_or_die(config.dataset, val);
		else if (key == "--n-clusters")
			valid_n_clusters_or_die(config.n_clusters, val);
		else if (key == "--n-iters")
			valid_n_iters_or_die(config.n_iters, val);
		else if (key == "--output")
			valid_output_or_die(config.output, val);
		else
			die("fatal: flag ", key, " not recognized.");
	}

	// Verify that all parameters have been populated.
	if (config.dataset.empty())
		die("fatal: --dataset <path> not specified.");
	if (config.output.empty())
		die("fatal: --output <path> not specified.");
	if (config.n_clusters == 0)
		die("fatal: --n-clusters <unsigned> not specified.");
	if (config.n_iters == 0)
		die("fatal: --n-iters <unsigned> not specified.");

	return config;
}

void print_config(const config_t& config)
{
	cout << "Printing execution's configuration:" << endl;
	cout << "\tDataset = " << config.dataset << endl;
	cout << "\tClusters = " << config.n_clusters << endl;
	cout << "\tIterations = " << config.n_iters << endl;
}

points_t read_dataset(const string& path, const uint32_t& n_dims)
{
	ifstream ifs(path, ios::binary);

	// Read the number of points in the dataset.
	uint32_t n_points;
	ifs.read((char*)&n_points, sizeof(uint32_t));

	points_t points;
	points.reserve(n_points);

	// A buffer to read each point before writing to @points.
	vector<float> buffer(n_dims);
	// The i-th point we are reading from the file.
	uint32_t c_point = 0;

	// Read the points.
	while (ifs.read((char*)buffer.data(), 100 * sizeof(float)))
		points.push_back(point_t(++c_point, buffer));

	ifs.close();

	return points;
}

void write_knng(const knng_t& knng, const uint32_t& k, const string& path)
{
	ofstream ofs(path, ios::out | ios::binary);

	for (size_t c_point = 0; c_point < knng.size(); ++c_point) {
		// The knn of the current point.
		auto const &knn = knng[c_point];

		// Write the knn of the current point.
		ofs.write(reinterpret_cast<char const *>(&knn[0]), k * sizeof(uint32_t));
	}

	ofs.close();
}
