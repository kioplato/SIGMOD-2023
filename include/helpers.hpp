#pragma once

#include <fstream>
#include <string>
#include <cstdint>
#include <cmath>
#include "point.hpp"

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
