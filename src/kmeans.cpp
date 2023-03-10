#include <omp.h>
#include <cstdint>
#include <cmath>
#include <vector>
#include <limits>
#include "kmeans.hpp"

using namespace std;

/** Private functions. ********************************************************/

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
 * @brief Initialize K-Means algorithm with the number of
 * clusters to create and the number of iterations to perform.
 *
 * @param n_clusters The number of clusters to create.
 * @param n_iters The maximum number of iterations to perform.
 */
kmeans_t::kmeans_t(uint32_t n_clusters, uint32_t n_iters)
: _n_clusters(n_clusters), _n_iters(n_iters)
{
	// Empty.
}

/*
 * @brief Perform K-Means clustering on a set of points.
 *
 * The number of iterations to perform and the number of
 * clusters to create has been provided in the constructor.
 *
 * @param points The points to cluster.
 *
 * @return Void. Each point gets assigned to a cluster.
 */
void kmeans_t::run(vector<point_t>& points)
{
	// The number of points we need to cluster.
	_n_points = points.size();
	// The n-dimensional space points live.
	_n_dims = points.front().size();

	/*
	 * Initialize clusters.
	 *
	 * Each cluster is initialized to a different point.
	 * Therefore we need to store which points have been used such that
	 * no two clusters are initialized with the same point. That means
	 * that they would share the same centroid, and in essense we would
	 * have duplicate clusters.
	 */

	// The IDs of the used points.
	vector<uint32_t> used_points;

	// Iterate over the cluster IDs and initialize each cluster.
	for (uint32_t c_cluster = 1; c_cluster <= _n_clusters;) {
		// Pick a random point to initialize current cluster.
		uint32_t index = rand() % _n_points;

		// Retry if the same point has been used as centroid for another cluster.
		if (find(used_points.begin(), used_points.end(), index) != used_points.end())
			continue;

		// Create a cluster with this point as centroid.
		cluster_t cluster(c_cluster, points[index]);
		// Add point to cluster.
		cluster.add_member(points[index]);
		// Assign cluster to point. The point now belongs to a cluster.
		points[index].set_cluster(c_cluster);

		// Mark the used point's ID.
		used_points.push_back(index);
		// Store the cluster in the vector with the other clusters.
		_clusters.push_back(cluster);

		++c_cluster;
	}

	/*
	 * Start improving the clusters and update each point's cluster.
	 *
	 * We either stop if we can no longer improve the clusters,
	 * or if maximum iterations @_n_iters have been performed.
	 */

	for (uint32_t c_iter = 0; c_iter < _n_iters; ++c_iter)
	{
		// We stop when we can no longer improve any cluster.
		bool done = true;

		// Add all points to their nearest cluster.
		#pragma omp parallel for reduction(&&: done)
		for (point_t& point : points)
		{
			uint32_t curr_cluster_id = point.get_cluster();
			uint32_t best_cluster_id = find_nearest_cluster(point);

			// Update the point's cluster.
			if (curr_cluster_id == best_cluster_id)
				continue;

			point.cluster_id(best_cluster_id);

			// TODO: Remove point from @curr_cluster_id and add it
			// to @best_cluster_id. A race could occur where two
			// points need to be removed from the same cluster or
			// added to the same cluster.

			// At least one cluster has been improved.
			done = false;
		}

		// If no cluster was improved then we are done.
		if (done) return;

		/*
		 * Readd all the points to their clusters
		 * and recalculate each cluster's centroid.
		 */

		#pragma omp parallel
		{
			// Clear all existing clusters.
			#pragma omp for
			for (auto& cluster : _clusters)
				cluster.reset_members();

			// Add each point to the cluster it belongs.
			#pragma omp for
			for (const auto& point : points)
				// Cluster's index is its id - 1.
				clusters[point.get_cluster() - 1].add_member(point);

			#pragma omp for
			for (cluster_t& curr_cluster : _clusters)
				curr_cluster.recenter();
		}
	}
}
