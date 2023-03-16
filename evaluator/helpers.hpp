#pragma once

#include <iostream>
#include <vector>

using namespace std;

/**
 * @brief Print provided arguments to stderr stream and exit with error code 1.
 * Prefix the string with the program's name and fatal to describe the error.
 *
 * @param args_t The provided arguments to print before exiting.
 *
 * @return The program exits with error code 1.
 */
template<class... args_t>
inline void die(args_t... args)
{
	cerr << "[evaluator] fatal: ";
	(cerr << ... << args) << endl;

	exit(1);
}

/*
 * @brief Print provided arguments to stderr stream and exit with error code 1.
 * Prefix the string with the program's name and internal to describe the error.
 *
 * @param args_t The provided arguments to print before exiting.
 *
 * @return The program exits with error code 1.
 */
template<class... args_t>
inline void diei(args_t... args)
{
	cerr << "[evaluator] internal: ";
	(cerr << ... << args) << endl;

	exit(1);
}

/*
 * @brief Calculate the euclidean distance between two vectors. The returned
 * value can only be used for comparison with other euclidean distances and
 * it does not represent the actual euclidean distance between them.
 *
 * @param lhs The first vector to use.
 * @param rhs The second vector to use.
 *
 * @return Comparable only euclidean distance between @lhs and @rhs.
 */
inline double
euclidean_distance_aprox(const vector<float> &lhs, const vector<float> &rhs)
{
	// The aprox distance between the lhs and rhs (only for comparison).
	double distance = 0.0;

	// The dimensions of the vectors.
	uint32_t n_dims = 100;

	// Compute the aprox euclidean distance.
	for (uint32_t c_dim; c_dim < n_dims; ++c_dim)
		distance += (lhs[c_dim] - rhs[c_dim]) * (lhs[c_dim] - rhs[c_dim]);

	return distance;
}
