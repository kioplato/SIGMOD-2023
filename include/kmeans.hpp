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

	// The number of dimensions of each point.
	uint32_t _n_dims;

	// The number of points in the dataset.
	uint32_t _n_points;

	// The clusters.
	vector<cluster_t> _clusters;

	// Find the nearest cluster to the specified point @assortee.
	uint32_t _find_nearest_cluster_id(const point_t& assortee) const;

public:
	// Initialize with the number of clusters and number of iterations.
	kmeans_t(uint32_t n_clusters, uint32_t n_iters);

	// Perform k-means clustering.
	void run(vector<point_t>& points);
};
