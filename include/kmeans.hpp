#pragma once

#include <cstdint>
#include <vector>
#include "helpers.hpp"
#include "point.hpp"
#include "cluster.hpp"

using namespace std;

class kmeans_t {
	// The number of clusters.
	uint32_t _n_clusters;

	// The number of iterations to perform. May converge faster.
	uint32_t _n_iters;

	// All the points used in the clustering.
	vector<point_t>& _points;

	// The clusters.
	vector<cluster_t> _clusters;

public:
	// Initialize with the number of clusters and number of iterations.
	kmeans_t(uint32_t n_clusters, uint32_t n_iters, vector<point_t>& points);

	// Perform k-means clustering.
	void run();

	// Print the clusters.
	void print_clusters(ostream& outstream, string indent = "") const;

	/*
	 * @brief Print a vector of @points to the specified stream @ofs.
	 *
	 * @param ofs Where to print the point.
	 * @param points The points to print.
	 * @param indent The indentation level to print the point.
	 *
	 * @return None.
	 */
	void print_points(ostream& outstream, string indent = "") const;
};
