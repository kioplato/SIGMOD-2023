#include <iostream>
#include <algorithm>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "knng.hpp"
#include "point.hpp"
#include "cluster.hpp"
#include "helpers.hpp"
#include "kmeans.hpp"

using namespace std;

/**
 * @brief Improve the k nearest neighbors of a point.
 * They are stored in a priority queue. We need to maintain k total items in
 * the queue as this reduces complexity from O(log(n)) to O(log(k)).
 *
 * @param point The point whose nearest neighbors we are improving.
 * @param k The number of nearest neighbors to maintain.
 * @param candidates The next batch of candidates from the next cluster.
 * @param nearest_neighbors The k nearest neighbors from furthest to nearest.
 *
 * @return None. We improve @nearest_neighbors in place.
 */
static inline void
improve_knn_of_point(const point_t& point, uint32_t k, const point_cptrs_t& candidates,
		priority_queue<pair<double, uint32_t>>& nearest_neighbors)
{
	for (size_t c_cand = 0; c_cand < candidates.size(); ++c_cand) {
		// Skip itself. A point isn't a neighbor of itself.
		if (candidates[c_cand]->id() == point.id())
			continue;

		const point_t& candidate = *(candidates[c_cand]);
		double distance = euclidean_distance_aprox(point, candidate);

		if (nearest_neighbors.size() < k)
			nearest_neighbors.push({distance, candidate.id() - 1});
		else if (nearest_neighbors.top().first > distance) {
			nearest_neighbors.pop();
			nearest_neighbors.push({distance, candidate.id() - 1});
		}
	}
}

/*
 * @brief Find the k nearest neighbors of the @point from the cluster it belongs.
 *
 * @param point The point to find its k nearest neighbors.
 * @param k How many nearest neighbors to find.
 *
 * @return Vector with k points, which are the knn of @point in its cluster.
 */
static inline vector<uint32_t>
knn_of_point(const point_t& point, uint32_t k)
{
	// Max heap. This way we know which is the furthest point from @id.
	priority_queue<pair<double, uint32_t>> nearest_neighbors;

	// Iterate from the nearest cluster towards the furthest.
	for (const cluster_t* cluster : point.clusters()) {
		const point_cptrs_t& candidates = cluster->points();

		improve_knn_of_point(point, k, candidates, nearest_neighbors);
	}

#ifdef VERBOSE
	// Each point must have exactly k neighbors.

	if (nearest_neighbors.size() < k) {
		printf("Found point with less than 100 nn.\n");
		point.print(cout, "", false);
		point.clusters()[0]->print(cout, "", false, false);
		die("fatal internal: point with ", nearest_neighbors.size(), " nn.");
	}
#endif

	// Write the nns of the current point @id.
	vector<uint32_t> knn;
	knn.reserve(k);

	while (!nearest_neighbors.empty()) {
		knn.push_back(nearest_neighbors.top().second);
		nearest_neighbors.pop();
	}

	/*
	 * @nearest_neighbors is max heap. The @knn contains the nearest
	 * neighbors from within the cluster but in reverse order i.e. the first
	 * neighbor is the furstest one from the k neighbors. We could reverse
	 * @knn but it's not mandatory because recall is the the true number of
	 * nearest neighbors in @knn despite their order.
	 */

	return knn;
}

knng_t create_knng(points_t& points, uint32_t k, uint32_t n_clusters,
		uint32_t n_iters, uint32_t n_nearest_clusters)
{
#ifdef VERBOSE
	cout << "Creating knng." << endl;
#endif

	/*
	 * Run K-Means clustering. We create @n_clusters. Each point belongs in
	 * one cluster, however, we also store each points @n_nearest_clusters.
	 * This way we can search in more clusters for the k-nn of a point.
	 */
	kmeans_t kmeans(n_clusters, n_iters, points, n_nearest_clusters);
	kmeans.run();

#ifdef VERBOSE
	/*
	 * Print the clusters without their participating points,
	 * since it would clutter the output way too much.
	 */
	kmeans.print_clusters(cout, "", false, false);
#endif

#ifdef VERBOSE
	cout << "Search exhaustively in each point's cluster." << endl;
#endif

	knng_t knng;
	knng.resize(points.size());

	#pragma omp parallel for
	for (size_t c_point = 0; c_point < points.size(); ++c_point)
		knng[c_point] = knn_of_point(points[c_point], k);

	return knng;
}
