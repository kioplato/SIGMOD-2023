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
	points_t& _points;

	// The clusters.
	clusters_t _clusters;

	// The number of nearest clusters each point will belong.
	uint32_t _n_nearest_clusters;

public:
	/**
	 * @brief Initialize the K-Means algorithm with how many clusters to
	 * create and how many iterations to perform while not converging.
	 *
	 * @param n_clusters The number of clusters to create.
	 * @param n_iters The max number of iterations to perform.
	 * @param points The points to cluster.
	 */
	kmeans_t(uint32_t n_clusters, uint32_t n_iters, points_t& points, uint32_t n_nearest_clusters);

	/**
	 * @brief Perform K-Means clustering. Create @_n_clusters clusters
	 * and perform @_n_iters number of iterations while not converging.
	 *
	 * @return None.
	 */
	void run();

	/**
	 * @brief Print the clusters to the specified stream @outstream.
	 *
	 * @param outstream The stream to write the clusters.
	 * @param indent The base indentation level to print the clusters.
	 * @param print_points Whether or not to also print participating points.
	 *
	 * @return None.
	 */
	void print_clusters(ostream& outstream, string indent, bool print_points) const;

	/**
	 * @brief Print the @_points to the specified stream @outstream.
	 *
	 * @param ofs Where to print the point.
	 * @param points The points to print.
	 * @param indent The indentation level to print the point.
	 *
	 * @return None.
	 */
	void print_points(ostream& outstream, string indent) const;
};
