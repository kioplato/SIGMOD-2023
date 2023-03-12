#include <cstdint>
#include <vector>
#include "point.hpp"

using namespace std;

/*
 * @brief Calculate the knng of the @points.
 *
 * The knng is stored as a vector where each item is the point with ID the
 * same as its index. Its nearest neighbors are stored as a nested vector.
 *
 * @param points The points to use for the knng construction.
 */
vector<vector<uint32_t>> create_knng(const vector<point_t>& points, uint32_t k);
