#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "helpers.hpp"
#include "input-output.hpp"

using namespace std;

/**
 * @brief Find the true k nearest neighbors of @query point.
 *
 * @param points All the points.
 * @param query The point to find its true @k nearest neighbors.
 * @param k The number of true nearest neighbors to find.
 *
 * @return The indices of the true k nearest neighbors to @query.
 */
static inline vector<uint32_t>
knn_of_point(const vector<vector<float>>& points,
		const vector<float>& query, const uint32_t k)
{
	// Max heap containing the nearest neighbors.
	priority_queue<pair<double, uint32_t>> nearest_neighbors;

	for (size_t c_point = 0; c_point < points.size(); ++c_point) {
		double distance = euclidean_distance_aprox(query, points[c_point]);

		if (nearest_neighbors.size() < k)
			nearest_neighbors.push({distance, c_point});
		else if (nearest_neighbors.top().first > distance) {
			nearest_neighbors.pop();
			nearest_neighbors.push({distance, c_point});
		}
	}

	// The indices of the k nearest points to @query.
	vector<uint32_t> knn;

	// Add the next closest point.
	while (!nearest_neighbors.empty()) {
		knn.emplace_back(nearest_neighbors.top().second);
		nearest_neighbors.pop();
	}

	/*
	 * Reverse the neighbors because @nearest_neighbors is max heap.
	 * This means that the root is the furthest neighbor from the k.
	 * Therefore the @knn vector contains the neighbors in reverse order.
	 */
	reverse(knn.begin(), knn.end());

	return knn;
}

/**
 * @brief Compute the true knn of the points with indices in @points specified
 * in @true_indices. Store them in a vector of vectors of indices and return.
 *
 * @param points All the points of the dataset.
 * @param true_indices The indices of the points in @points.
 * @param k The number of true nearest neighbors to find for each point.
 *
 * @return The true k nearest neighbors of the specified points.
 */
static inline vector<vector<uint32_t>>
sample_true_knng(vector<vector<float>> points,
		const vector<uint32_t>& true_indices, const uint32_t& k)
{
	// Here we will store the sampled true knng.
	vector<vector<uint32_t>> sampled_true_knng(true_indices.size());

	#pragma omp parallel for
	for (uint32_t c_index = 0; c_index < true_indices.size(); ++c_index) {
		uint32_t index = true_indices[c_index];
		vector<float> point = points[index];

		vector<uint32_t> true_knn = knn_of_point(points, point, k + 1);
		// Remove itself from the k + 1 nearest neighbors.
		true_knn.erase(true_knn.begin());

		sampled_true_knng[c_index] = true_knn;
	}

	return sampled_true_knng;
}

int main(int argc, char **argv)
{
	// The path to the dataset whose true knng we want to compute.
	string dataset_path;
	// The sample size of the true knng we want to create.
	uint32_t n_samples = 0;
	// Where to write the true knng sample we will compute.
	string output_path;

	for (uint32_t c_arg = 1; c_arg < argc; ++c_arg) {
		string key(argv[c_arg]);

		if (c_arg == argc - 1)
			die("flag ", key, " isn't followed by a value.");

		string val(argv[++c_arg]);

		if (key == "--dataset-path" && !dataset_path.empty())
			die("--dataset-path flag already provided.");
		else if (key == "--dataset-path")
			dataset_path = val;
		else if (key == "--n-samples" && n_samples != 0)
			die("--n-samples flag already provided.");
		else if (key == "--n-samples")
			n_samples = atoll(val.c_str());
		else if (key == "--output-path" && !output_path.empty())
			die("--output-path flag already provided.");
		else if (key == "--output-path")
			output_path = val;
		else
			die(key, " flag not recognized");
	}

	// Verify that command-line arguments are populated.
	if (dataset_path.empty())
		die("--dataset-path flag not provided.");
	if (output_path.empty())
		die("--output-path flag not provided.");

	// Check that dataset file exists and output file does not.
	// TODO.

	// The number of dimensions of each point.
	const uint32_t n_dims = 100;
	// The number of nearest neighbors to compute.
	const uint32_t k = 100;

	// Read the dataset.
	vector<vector<float>> points = read_dataset(dataset_path, n_dims);

	if (points.size() < n_samples)
		die("--n-samples cannot be greater than dataset's cardinality.");

	// Pick the indices of points to sample.
	vector<uint32_t> true_indices(points.size());
	iota(true_indices.begin(), true_indices.end(), 0);
	// If --n-samples wasn't specified we generate the complete true knng.
	if (n_samples != 0) {
		random_device rd;
		mt19937 gen(rd());
		shuffle(true_indices.begin(), true_indices.end(), gen);
		true_indices.resize(n_samples);
	}

	// Compute the true knng of the sampled point indices @true_indices.
	vector<vector<uint32_t>> true_knng = sample_true_knng(points, true_indices, k);

	// The sampled knng points must be equal to @true_indices and @n_samples.
	if (true_knng.size() != true_indices.size())
		die("sampled true knng size differs from true indices size.");

	// Each sampled point must have exactly k nearest neighbors.
	#pragma omp parallel for
	for (vector<uint32_t> knn : true_knng)
		if (knn.size() != k)
			die("sampled true knng does not have ", k, " nearest neighbors.");

	// Write the sampled true knng to the @output_path.
	write_true_knng(output_path, true_knng, true_indices);

	return EXIT_SUCCESS;
}
