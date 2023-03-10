#include <omp.h>
#include <cstdint>
#include <cmath>
#include <vector>
#include <limits>
#include "kmeans.hpp"

using namespace std;

/** Private functions. ********************************************************/

/*
 *
 */
void kmeans_t::_clear_clusters_members()
{
	for (auto& cluster : _clusters)
		cluster.reset_members();
}

/*
 * @brief Find the nearest cluster of @assortee.
 *
 * @param assortee The point to find its nearest cluster from @_clusters.
 *
 * @return The ID of the @assortee's nearest cluster.
 */
uint32_t kmeans_t::_find_nearest_cluster_id(const point_t& assortee) const
{
	// Initialize the best cluster.
	double best_distance = euclidean_distance(_clusters[0].centroid(), assortee);
	uint32_t best_cluster_id = _clusters[0].id();

	// Iterate over the rest clusters and find the nearest one.
	#pragma omp parallel
	{
		double best_distance_thr = best_distance;
		uint32_t best_cluster_id_thr = best_cluster_id;

		#pragma omp for
		for (const auto& cluster : _clusters)
		{
			double distance = euclidean_distance(cluster.centroid(), assortee);

			if (distance < best_distance_thr)
			{
				best_distance_thr = distance;
				best_cluster_id_thr = cluster.id();
			}
		}

		#pragma omp critical
		if (best_distance_thr < best_distance)
		{
			best_distance = best_distance_thr;
			best_cluster_id = best_cluster_id_thr;
		}
	}

	return best_cluster_id;
}

/** Public functions **********************************************************/

/*
 * @brief Perform K-Means clustering on a set of points.
 */
void kmeans_t::run(vector<point_t>& points, uint32_t n_clusters, uint32_t n_iters)
{
	_n_points = points.size();
	_n_dims = points.front().size();

	/* Initialize clusters. */
	vector<uint32_t> used_points;

	for (uint32_t c_cluster = 1; c_cluster <= _n_clusters; ++c_cluster)
	{
		while (true)
		{
			uint32_t index = rand() % _n_points;

			// Retry if the same point has been used as centroid for another cluster.
			if (find(used_points.begin(), used_points.end(), index) != used_points.end())
				continue;

			// Create a cluster with this point as centroid.
			cluster_t cluster(c_cluster, points[index]);
			// Assign point to the cluster.
			points[index].set_cluster(c_cluster);
			cluster.add(points[index]);

			clusters.push_back(cluster);
			used_ids.push_back(index);

			break;
		}
	}

	for (uint32_t c_iter = 0; c_iter < _n_iters; ++c_iter)
	{
		// We stop when we can no longer improve any cluster.
		bool done = true;

		// Add all points to their nearest cluster.
		#pragma omp parallel for reduction(&&: done)
		for (auto& point : points)
		{
			int curr_cluster_id = point.get_cluster();
			int best_cluster_id = find_nearest_cluster(point);

			if (curr_cluster_id != best_cluster_id)
			{
				point.cluster_id(best_cluster_id);
				done = false;
			}
		}

		// Clear all existing clusters.
		clear_clusters_members();

		// Reassign points to their new clusters.
		for (const auto& point : points)
			// Cluster's index is its id - 1.
			clusters[point.get_cluster() - 1].add(point);

		// Recalculating the center of each cluster.
		for (auto& curr_cluster : _clusters)
			curr_cluster.recenter();

		if (done) break;
	}
}
