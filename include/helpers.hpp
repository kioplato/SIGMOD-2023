#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cmath>
#include "point.hpp"

// Various typedefs to shorten the type names.
typedef vector<point_t> points_t;
typedef vector<point_t*> point_ptrs_t;
typedef vector<const point_t*> point_cptrs_t;
typedef vector<cluster_t> clusters_t;
typedef vector<vector<uint32_t>> knng_t;

/**
 * @brief Compute the euclidean distance of two points.
 *
 * @param point1 The first point to use for euclidean distance.
 * @param point2 The second point to use for euclidean distance.
 *
 * @return The euclidean distance between @point1 and @point2.
 */
inline double
euclidean_distance_aprox(const point_t& point1, const point_t& point2)
{
	double distance = 0;

	const vector<float>& coords1 = point1.coords();
	const vector<float>& coords2 = point2.coords();
	uint32_t n_dims = point1.coords().size();

	if (n_dims == 1)
		return abs(coords1[0] - coords2[0]);

	for (uint32_t c_dim = 0; c_dim < n_dims; ++c_dim)
		distance += (coords1[c_dim] - coords2[c_dim]) * (coords1[c_dim] - coords2[c_dim]);

	return distance;
}

/**
 * @brief Print provided arguments to stderr stream and exit with error code 1.
 * Prefix the provided arguments with the program's name and fatal.
 * Use this function to describe an error that is expected.
 *
 * @param args_t The provided arguments to print before exiting.
 *
 * @return The program exits with with error code 1.
 */
template<class... args_t>
inline void die(args_t... args)
{
	cerr << "[knng-clusters] fatal: ";
	(cerr << ... << args) << endl;

	exit(1);
}

/**
 * @brief Print provided arguments to stderr stream and exit with error code 1.
 * Prefix the provided arguments with the program's name and internal.
 * Use this function to describe an error that should not occur.
 *
 * @param args_t The provided arguments to print before exiting.
 *
 * @return The program exits with with error code 1.
 */
template<class... args_t>
inline void diei(args_t... args)
{
	cerr << "[knng-clusters] internal: ";
	(cerr << ... << args) << endl;

	exit(1);
}
