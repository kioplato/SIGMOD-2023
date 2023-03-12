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
	uint32_t n_dims = point1.n_dims();

	if (n_dims == 1)
		return abs(point1[0] - point2[0]);

	for (uint32_t c_dim = 0; c_dim < n_dims; ++c_dim)
		distance += pow(point1[c_dim] - point2[c_dim], 2);

	return distance;
}

inline void
points_print(const vector<point_t>& points, string path)
{
	ofstream ofs(path, ios::out | ios::trunc);

	for (const point_t& point : points)
		point.print(ofs);

	ofs.close();
}
