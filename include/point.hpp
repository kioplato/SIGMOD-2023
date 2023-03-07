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
	const cluster_t* _cluster;

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

	/*
	 * @brief Get a pointer to the point's cluster.
	 *
	 * @return None.
	 */
	const cluster_t* cluster() const;

	/*
	 * @brief Set the cluster in which the point belongs.
	 *
	 * @param cluster Pointer to the cluster that the point belongs.
	 *
	 * @return None.
	 */
	void cluster(const cluster_t* cluster);

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
	void print(ostream& outstream, string indent) const;
};
