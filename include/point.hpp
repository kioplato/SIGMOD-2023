#pragma once

#include <cstdint>
#include <vector>

using namespace std;

class cluster_t;

/* Class storing a 2-dimensional point. */
class point_t {
	// The identifier of the point.
	// A point with _id = 0 means its undefined.
	uint32_t _id = 0;

	// The id of the cluster the point belongs to.
	// A cluster with _cluster_id = 0 means its undefined.
	uint32_t _cluster_id = 0;

	// Pointer to the cluster the point belongs.
	const cluster_t& _cluster;

	// The coordinates of the point in the n-dimensions.
	vector<double> _coordinates;

public:
	// Initialize the point at the origin of the n_dims-dimensional space.
	point_t(size_t n_dims);

	// Initialize the point with its id and exact coordinates.
	point_t(uint32_t id, const vector<double>& coordinates);

	// Get the number of dimensions that point lives in.
	size_t n_dims() const;

	// Get the ID of the point.
	uint32_t id() const;

	// Get the id of the cluster that the point belongs.
	uint32_t cluster_id() const;

	// Set the cluster id of the point.
	void cluster_id(uint32_t id);

	const cluster_t& cluster() const;
	void cluster(const cluster_t& cluster);


	// Get the i-th dimension's value.
	double operator[] (size_t index) const;
	double& operator[] (size_t index);

};
