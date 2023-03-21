#pragma once

#include <fstream>
#include <cstdint>
#include <vector>
#include "typedefs.hpp"

using namespace std;

class point_t {
	// The identifier of the point.
	uint32_t _id;

	// Pointer to the cluster the point belongs.
	cluster_t* _cluster;

	// Pointers to the point's nearest clusters.
	cluster_ptrs_t _clusters;

	// The coordinates of the point in the n-dimensions.
	vector<float> _coordinates;

public:
	/*
	 * @brief Initialize the point with an ID and its coordinates.
	 *
	 * @param id The id of the point. Should be unique.
	 * @param coordinates Where the point lives, its location.
	 */
	point_t(const uint32_t& id, const vector<float>& coordinates);

	/*
	 * @brief Get the ID of the point.
	 *
	 * @return The ID of the point.
	 */
	uint32_t id() const;

	/**
	 * @brief Set the point's nearest cluster.
	 *
	 * @param cluster Pointer to the cluster to set as nearest cluster.
	 *
	 * @return None.
	 */
	void cluster(cluster_t* cluster);

	/**
	 * @brief Get this point's nearest cluster.
	 *
	 * @return The address of this point's nearest cluster.
	 */
	cluster_t* cluster();

	/*
	 * @brief Get a pointer to the point's cluster.
	 *
	 * @return None.
	 */
	const cluster_ptrs_t& clusters() const;

	/*
	 * @brief Set the cluster in which the point belongs.
	 *
	 * @param cluster Pointer to the cluster that the point belongs.
	 *
	 * @return None.
	 */
	void clusters(const cluster_ptrs_t& clusters);

	/*
	 * @brief Get the coordinates of the point.
	 *
	 * @return None.
	 */
	const vector<float>& coords() const;

	/*
	 * @brief Print the point to the specified stream.
	 *
	 * @param ofs The stream to print the point.
	 * @param indent The indentation level to print the point.
	 *
	 * @return None.
	 */
	void print(ostream& outstream, string indent, bool print_coords) const;
};
