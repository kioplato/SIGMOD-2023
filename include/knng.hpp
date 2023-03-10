#include <cstdint>
#include <vector>
#include "point.hpp"

using namespace std;

/*
 * @brief Find the k nearest neighbors of the @point from the cluster it belongs.
 *
 * @param point The point to find its k nearest neighbors.
 * @param k How many nearest neighbors to find.
 *
 * @return Vector with k points, which are the knn of @point in its cluster.
 */
vector<uint32_t> knn_of_point(const point_t& point, uint32_t k);

vector<vector<>>
