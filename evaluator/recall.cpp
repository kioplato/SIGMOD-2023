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
 * @brief Compute the recall of a single point.
 *
 * @param eval_knn The point's k nearest neighbors to evaluate.
 * @param true_knn The point's true k nearest neighbors.
 * @param k The number of nearest neighbors per point.
 *
 * @return The recall score of the provided point.
 */
static inline double
recall_of_point(const vector<uint32_t>& eval_knn,
		const vector<uint32_t>& true_knn, const uint32_t& k)
{
	// Both eval and true knng must have k nearest neighbors.
	if (eval_knn.size() != k || true_knn.size() != k)
		die("eval and true knng must have k nearest neighbors.");

	// The number of correct nearest neighbors of this point.
	uint32_t correct_nn = 0;

	for (uint32_t point_index : true_knn)
		if (find(eval_knn.begin(), eval_knn.end(), point_index) != eval_knn.end())
			++correct_nn;

	return correct_nn / (double)k;
}

/**
 * @brief Compute the recall score of @eval_knng using @true_knng as groundtruth.
 *
 * @param eval_knng The knng to evaluate its recall score.
 * @param true_knng The true knng to use as ground truth.
 * @param true_indices The indices of the @true_knng sample points.
 * @param k The number of nearest neighbors per point.
 *
 * @return The recall score of @eval_knng.
 */
static inline double
recall_of_points(const vector<vector<uint32_t>>& eval_knng,
		const vector<vector<uint32_t>> &true_knng,
		const vector<uint32_t> &true_indices, const uint32_t& k)
{
	double recall = 0.0;

	#pragma omp parallel for reduction(+ : recall)
	for (uint32_t i = 0; i < true_indices.size(); ++i) {
		uint32_t true_index = true_indices.at(i);
		vector<uint32_t> eval_knn = eval_knng.at(true_index);
		vector<uint32_t> true_knn = true_knng.at(i);

		// Sum up all recall scores.
		recall += recall_of_point(eval_knn, true_knn, k);
	}

	// Calculate average recall score.
	recall /= true_indices.size();

	return recall;
}

int main(int argc, char **argv)
{
	// The path to the true knng (the ground truth).
	string true_knng_path;
	// The path to the knng for evaluation.
	string eval_knng_path;

	// Iterate over argv and index the command-line arguments.
	for (uint32_t c_arg = 1; c_arg < argc; ++c_arg) {
		string arg(argv[c_arg]);

		if (arg == "--true-knng-path" && !true_knng_path.empty())
			die("--true-knng-path flag already provided.");
		else if (arg == "--true-knng-path")
			true_knng_path = string(argv[++c_arg]);
		else if (arg == "--eval-knng-path" && !eval_knng_path.empty())
			die("--eval-knng-path flag already provided.");
		else if (arg == "--eval-knng-path")
			eval_knng_path = string(argv[++c_arg]);
		else
			die(arg, " flag not recognized.");
	}

	// Verify that command-line arguments are populated.
	if (true_knng_path.empty())
		die("--true-knng-path flag not provided.");
	if (eval_knng_path.empty())
		die("--eval-knng-path flag not provided.");

	// Check that both files exist.
	// TODO.

	cout << "Configuration:" << endl;
	cout << "\tTrue knng path = " << true_knng_path << endl;
	cout << "\tEval knng path = " << eval_knng_path << endl;

	// The number of nearest neighbors per point.
	const uint32_t k = 100;

	// Load the true knng (groundtruth).
	auto [true_knng, true_indices] = load_true_knng(true_knng_path, k);

	cout << "True knng contains " << true_knng.size() << " sample points." << endl;

	// Load the knng for evaluation.
	vector<vector<uint32_t>> eval_knng = load_knng(eval_knng_path, k);

	cout << "Eval knng contains " << eval_knng.size() << " points." << endl;

	// Compute the recall of @eval_knng using @true_knng.
	double recall = recall_of_points(eval_knng, true_knng, true_indices, k);

	cout << "Recall score: " << recall << endl;

	return EXIT_SUCCESS;
}
