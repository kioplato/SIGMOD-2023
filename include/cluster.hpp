#pragma once

#include <iostream>
#include "point.hpp"
#include "helpers.hpp"

using namespace std;

/* A cluster. */
class cluster_t {
	// The id of the cluster.
	uint32_t _cluster_id;

	// The center of the cluster.
	point_t _centroid;

	// The points in this cluster.
	// TODO: Use a map for guaranteed O(log(n)) or
	// unordered map with Szudzik's point hash function.
	point_cptrs_t _points;

public:
	// Initialize the cluster with its id and centroid point.
	cluster_t(uint32_t cluster_id, const vector<float>& centroid);

	// The id of this cluster.
	uint32_t id() const;

	/*
	 * Get the centroid of the cluster.
	 */
	const point_t& centroid() const;

	/*
	 * Change centroid to the specified one.
	 */
	void centroid(const point_t& centroid);

	/*
	 * @brief Recalculate the cluster's center and update centroid.
	 */
	void recenter();

	/*
	 * Add a point to the cluster.
	 */
	void add_point(const point_t* point);

	/*
	 * Remove a point from the cluster using its id.
	 */
	bool remove_point(uint32_t point_id);

	// Get the cluster's points.
	const point_cptrs_t& points() const;

	/*
	 * @brief Remove all points from this cluster.
	 *
	 * @return None.
	 */
	void clear();

	/*
	 * @brief Print the cluster ID, its centroid and its member points.
	 *
	 * @return None.
	 */
	void print(ostream& outstream, string indent, bool print_points) const;
};
