#include <fstream>
#include <string>
#include <vector>
#include "point.hpp"

using namespace std;

/*
 * @brief Reading binary data vectors. Raw data stored as a (N x 100) float.
 *
 * @param path Path to the file that contains the dataset in binary format.
 * @param n_dims The dimension of each point in @path to read.
 *
 * @return The points
 */
vector<point_t> read_dataset(const string& path, const uint32_t& n_dims);

/*
 * @brief Save knng in binary format (uint32_t) with the specified name.
 *
 * @param knng Is a (n_points * 100) shape 2-D vector. It stores the i-th
 * point's k nearest neighbors. All indexes refer to the point with the same id.
 * @param k The number of nearest neighbors per point.
 * @param path Where to write the knng.
 *
 * @return None.
 */
void write_knng(const vector<vector<uint32_t>>& knng, uint32_t k, string path);
