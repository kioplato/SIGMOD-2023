#pragma once

#include <cstdint>
#include <vector>
#include "helpers.hpp"

using namespace std;

/* Class storing a 2-dimensional point. */
class point_t {
	// The identifier of the point.
	// A point with _id = 0 means its undefined.
	uint32_t _id = 0;

	// The id of the cluster the point belongs to.
	// A cluster with _cluster_id = 0 means its undefined.
	uint32_t _cluster_id = 0;

	// The coordinates of the point in the n-dimensions.
	const vector<double> _coordinates;

public:
	// Initialize the point at the origin of the n_dims-dimensional space.
	point_t(size_t n_dims);

	// Initialize the point with its id and exact coordinates.
	point_t(uint32_t id, const vector<double>& coordinates);

	// Get the number of dimensions that point lives.
	size_t n_dims() const;

	// Get the id of the cluster that the point belongs.
	uint32_t cluster_id() const;

	// Get the i-th dimension's value.
	double operator[] (size_t index) const;
	double& operator[] (size_t index);


	// Set the cluster id of the point.
	void cluster_id(uint32_t id);
};

/* A cluster. */
class cluster_t {
	// The id of the cluster.
	uint32_t _cluster_id;

	// The center of the cluster.
	point_t _centroid;

	// The points in this cluster.
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

	// Recalculate the clusters center and update centroid.
	void recenter();
};

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

	// Clear all the clusters.
	void _clear_clusters_members();

	// Find the nearest cluster to the specified point @assortee.
	uint32_t _find_nearest_cluster_id(const point_t& assortee) const;

public:
	// Initialize with the number of clusters and number of iterations.
	kmeans_t(uint32_t n_clusters, uint32_t n_iters);

	// Perform k-means clustering.
	void run(vector<point_t>& points);
};
