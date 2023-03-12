#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include "input-output.hpp"
#include "helpers.hpp"

using namespace std;

vector<point_t> read_dataset(const string& path, const uint32_t& n_dims)
{
	ifstream ifs(path, ios::binary);

	// Read the number of points in the dataset.
	uint32_t n_points;
	ifs.read((char*)&n_points, sizeof(uint32_t));

	vector<point_t> points;
	points.reserve(n_points);

	// A buffer to read each point before writing to @points.
	vector<float> buffer(n_dims);
	// The i-th point we are reading from the file.
	uint32_t c_point = 0;

	// Read the points.
	while (ifs.read((char*)buffer.data(), n_dims * sizeof(float)))
		points.push_back(point_t(c_point, buffer));

	ifs.close();

	return points;
}

void write_knng(const vector<vector<uint32_t>>& knng, uint32_t k, string path)
{
	ofstream ofs(path, ios::out | ios::binary);

	for (size_t c_point = 0; c_point < knng.size(); ++c_point) {
		// The knn of the current point.
		auto const &knn = knng[c_point];

		// Write the knn of the current point.
		ofs.write(reinterpret_cast<char const *>(&knn[0]), k * sizeof(uint32_t));
	}

	ofs.close();
}
