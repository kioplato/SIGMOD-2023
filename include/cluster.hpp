#pragma once

#include <iostream>
#include "point.hpp"

using namespace std;

/* A cluster. */
class cluster_t {
	// The id of the cluster.
	uint32_t _cluster_id;

	// The center of the cluster.
	point_t _centroid;

	// The points in this cluster.
	// TODO: Use a map for guaranteed O(log(n)) or unordered map with
	// Szudzik's point hash function.
	vector<point_t> _points;

public:
	// Initialize the cluster with its id and centroid point.
	cluster_t(uint32_t cluster_id, const point_t& centroid);

	// The id of this cluster.
	uint32_t id() const;

	// Add a point to the cluster.
	void add_member(const point_t& point);

	// Remove a point from the cluster using its id.
	bool remove_member(uint32_t point_id);

	// The number of points in this cluster.
	size_t size() const;

	// Get a specific point by index.
	const point_t& operator[] (size_t index) const;

	// Get the centroid of the cluster.
	const point_t& centroid() const;

	// Change centroid to the specified one.
	void centroid(const point_t& centroid);

	// Remove all points from this cluster.
	void clear_members();

	// Get the cluster's points.
	const vector<point_t>& members() const;

	// Recalculate the cluster's center and update centroid.
	void recenter();
};
