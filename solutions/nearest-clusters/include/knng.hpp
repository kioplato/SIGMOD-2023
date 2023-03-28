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
 * @param k The dimension each point lives in.
 * @param n_clusters The number of clusters to create.
 * @param n_iters The maximum number of iterations to perform.
 *
 * @return Each points nearest neighbors. Indexes of the top level vector
 * correspond to the index of each point. uint32_t numbers are the indexes
 * of each point's nearest neighbors. The index of each point correspond
 * to the order in which they were read from the dataset file.
 */
knng_t create_knng(points_t& points, uint32_t k, uint32_t n_clusters,
		uint32_t n_iters, uint32_t n_nearest_clusters);
