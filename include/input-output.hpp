#include <fstream>
#include <string>
#include <vector>
#include "point.hpp"
#include "helpers.hpp"

using namespace std;

struct config_t {
	// The path to the dataset.
	string dataset;

	// The path to the output file.
	string output;

	// The number of clusters to create.
	uint32_t n_clusters = 0;

	// The number of iterations to perform while clusters keep improving.
	uint32_t n_iters = 0;

	uint32_t n_nearest_clusters = 0;
};

/**
 * @brief Parse command-line's arguments, check correctness and
 * properly index them inside the structure config_t.
 *
 * @argc The number of arguments in @argv.
 * @argv Array of c-strings containing the command-line arguments.
 *
 * @return config_t object containing this execution's specifics.
 * i.e. Dataset path, number of clusters and number of iterations.
 */
config_t parse_argv(uint32_t argc, char **argv);

/**
 * @brief Print this execution's configuration to stdout.
 *
 * @config The configuration to print.
 *
 * @return None.
 */
void print_config(const config_t& config);

/**
 * @brief Reading binary data vectors. Raw data stored as a (N x 100) float.
 *
 * @param path Path to the file that contains the dataset in binary format.
 * @param n_dims The dimension of each point in @path to read.
 *
 * @return The points
 */
points_t read_dataset(const string& path, const uint32_t& n_dims);

/**
 * @brief Save knng in binary format (uint32_t) with the specified name.
 *
 * @param knng Is a (n_points * 100) shape 2-D vector. It stores the i-th
 * point's k nearest neighbors. All indexes refer to the point with the same id.
 * @param k The number of nearest neighbors per point.
 * @param path Where to write the knng.
 *
 * @return None.
 */
void write_knng(const knng_t& knng, const uint32_t& k, const string& path);
