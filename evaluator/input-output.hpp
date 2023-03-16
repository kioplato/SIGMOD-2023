#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "helpers.hpp"

using namespace std;

/**
 * @brief Read the binary stored dataset points from @path.
 *
 * @param path Where the binary stored points are in the filesystem.
 * @param n_dims The number of dimensions each point has.
 *
 * @return Vector containing the points coordinates.
 */
inline vector<vector<float>>
read_dataset(const string& path, const uint32_t& n_dims)
{
	ifstream ifs(path, ios::binary);

	// Read the number of points in the dataset.
	uint32_t n_points;
	ifs.read((char*)&n_points, sizeof(uint32_t));

	vector<vector<float>> points;
	points.reserve(n_points);

	// A buffer to read each point before writing to @points.
	vector<float> buffer(n_dims);
	// The i-th point we are reading from the file.
	uint32_t c_point = 0;

	// Read the points.
	while (ifs.read((char*)buffer.data(), n_dims * sizeof(float)))
		points.push_back(buffer);

	ifs.close();

	return points;
}

/**
 * @brief Write the sampled true knng to the specified @path file.
 *
 * @param path The output file path.
 * @param true_knng The sampled true knng to write.
 * @param true_indices The indices of the sampled points in @true_knng.
 *
 * @return None.
 */
inline void
write_true_knng(const string& path, const vector<vector<uint32_t>>& true_knng,
		const vector<uint32_t>& true_indices)
{
	// Open the output file to write the sampled true knng.
	ofstream ofs(path, ios::out | ios::binary);
	if (!ofs.is_open())
		die("failed to open the output file ", path);

	// The number of points to write.
	const uint32_t n_points = true_indices.size();

	// Write the @n_points to the output file @path.
	ofs.write(reinterpret_cast<char const *>(&n_points), sizeof(uint32_t));

	// Write each point's index and its nearest neighbors.
	for (uint32_t i = 0; i < true_indices.size(); ++i) {
		// Write point's index.
		ofs.write(reinterpret_cast<const char*>(&true_indices[i]), sizeof(uint32_t));
		// Write point's nearest neighbors.
		ofs.write(reinterpret_cast<const char*>(&true_knng[i][0]), sizeof(uint32_t) * true_knng[i].size());
	}

	ofs.close();
}

/**
 * @brief Load the specified true knng at @path.
 *
 * @param path The path in the filesystem where the true knng to load is stored.
 * @param k The number of nearest neighbors stored.
 *
 * @return The nearest neighbors of the N stored points. The true knng can be a
 * sample of the true knng and therefore it also prefixes the actual index of
 * the point before its knn. Therefore two vectors are returned: the first is
 * the nearest neighbors of the N sampled points and the second is the indices
 * of the sampled points. We need the indices because we want to compare the
 * correct point from eval and true knngs.
 */
inline pair<vector<vector<uint32_t>>, vector<uint32_t>>
load_true_knng(const string &path, uint32_t k)
{
	// Open the file to read the true knng.
	ifstream ifs(path, ios::out | ios::binary);
	if (!ifs.is_open())
		die("failed to open true knng stored at ", path);

	// Read the number of points stored in the true knng sample.
	uint32_t n_points;
	ifs.read((char*)&n_points, sizeof(uint32_t));

	// Where we will store the true knng.
	vector<vector<uint32_t>> true_knng;
	true_knng.reserve(n_points);
	// The indices of the sampled points.
	vector<uint32_t> true_indices;
	true_indices.reserve(n_points);

	// Buffer to store the sample point's index.
	uint32_t index;
	// Buffer to store the k nearest neighbors indices read.
	vector<uint32_t> buffer(k);

	while (ifs.read((char*)&index, sizeof(uint32_t))) {
		true_indices.push_back(index);

		// Read the nearest neighbors of the current point.
		if (!ifs.read((char*)buffer.data(), sizeof(uint32_t) * k))
			die("true knng is missing the nn of the point with index ", index);

		true_knng.push_back(buffer);
	}

	ifs.close();

	return {true_knng, true_indices};
}

/**
 * @brief TODO.
 */
inline void
save_knng(const vector<vector<uint32_t>>& knng, const uint32_t& k, const string& path)
{
	ofstream ofs(path, ios::out | ios::binary);
	if (!ofs.is_open())
		die("failed to open ", path, " file to save knng.");

	for (size_t c_point = 0; c_point < knng.size(); ++c_point) {
		// The knn of the current point.
		const vector<uint32_t>& knn = knng[c_point];

		// Write the knn of the current point.
		ofs.write(reinterpret_cast<char const *>(&knn[0]), k * sizeof(uint32_t));
	}

	ofs.close();
}

/**
 * @brief Load the binary knng stored in the provided @path.
 *
 * @param knng Where to write the read knng.
 * @param k The number of nearest neighbors per point.
 * @param path Where the binary knng, that we need to load, is stored.
 *
 * @return None.
 */
inline vector<vector<uint32_t>>
load_knng(const string& path, const uint32_t k)
{
	ifstream ifs(path, ios::in | ios::binary);
	if (!ifs.is_open())
		diei("failed to open binary knng stored at ", path);

	// We will write the read knng here.
	vector<vector<uint32_t>> knng;

	// A buffer to read each point before writing to @.
	vector<uint32_t> buffer(k);

	while (ifs.read((char*)buffer.data(), k * sizeof(uint32_t)))
		knng.push_back(buffer);

	ifs.close();

	return knng;
}
