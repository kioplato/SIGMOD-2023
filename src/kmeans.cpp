#include <cstdint>
#include <vector>
#include <algorithm>
#include "kmeans.hpp"

using namespace std;

/*
 * @brief Initialize K-Means algorithm with the number of
 * clusters to create and the number of iterations to perform.
 *
 * @param n_clusters The number of clusters to create.
 * @param n_iters The maximum number of iterations to perform.
 */
kmeans_t::kmeans_t(uint32_t n_clusters, uint32_t n_iters, points_t& points)
: _n_clusters(n_clusters), _n_iters(n_iters), _points(points)
{
	// Empty.
}

/*
 * @brief Find the nearest cluster of @assortee.
 *
 * @param clusters The cluster to search.
 * @param assortee The point to find its nearest cluster from @_clusters.
 *
 * @return The memory address of the @assortee's nearest cluster.
 */
static inline const cluster_t*
_find_nearest_cluster(const clusters_t& clusters, const point_t& assortee)
{
	// Initialize the best cluster.
	double best_distance = euclidean_distance_aprox(clusters[0].centroid(), assortee);
	const cluster_t* best_cluster = &clusters[0];

	// Iterate over the rest clusters and find the nearest one.
	#pragma omp parallel
	{
		double best_distance_thr = best_distance;
		const cluster_t* best_cluster_thr = best_cluster;

		#pragma omp for nowait
		for (const cluster_t& cluster : clusters)
		{
			double distance = euclidean_distance_aprox(cluster.centroid(), assortee);

			if (distance < best_distance_thr)
			{
				best_distance_thr = distance;
				best_cluster_thr = &cluster;
			}
		}

		#pragma omp critical
		if (best_distance_thr < best_distance)
		{
			best_distance = best_distance_thr;
			best_cluster = best_cluster_thr;
		}
	}

	return best_cluster;
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
void kmeans_t::run()
{
	// The number of points we need to cluster.
	size_t n_points = _points.size();
	// The n-dimensional space the points live.
	size_t n_dims = _points.front().coords().size();

	/*
	 * Initialize clusters.
	 *
	 * Each cluster is initialized to a different point.
	 * Therefore we need to store which points have been used such that
	 * no two clusters are initialized with the same point. That means
	 * that they would share the same centroid, and in essense we would
	 * have duplicate clusters.
	 */

	/*
	 * Guarantee that the vector won't be reallocated.
	 * Otherwise the pointers won't point to the reallocated clusters.
	 */
	_clusters.reserve(_n_clusters);

	// The IDs of the used points.
	vector<uint32_t> used_points;

	// Iterate over the cluster IDs and initialize each cluster.
	for (uint32_t c_cluster = 1; c_cluster <= _n_clusters;) {
		// Pick a random point to initialize current cluster.
		uint32_t index = rand() % n_points;

		// Retry if the same point has been used as centroid for another cluster.
		if (find(used_points.begin(), used_points.end(), index) != used_points.end())
			continue;

		// Create a cluster with this point as centroid.
		_clusters.push_back({c_cluster, _points[index].coords()});

		// Add point to cluster.
		_clusters.back().add_point(&_points[index]);
		// Add cluster to point.
		_points[index].cluster(&_clusters.back());

		// Don't use the same point .
		used_points.push_back(index + 1);

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
		for (point_t& point : _points)
		{
			uint32_t curr_cluster_id = (point.cluster()) ? point.cluster()->id() : 0;
			const cluster_t* best_cluster = _find_nearest_cluster(_clusters, point);

			// Update the point's cluster if the current one isn't the best.
			if (curr_cluster_id == best_cluster->id())
				continue;

			point.cluster(best_cluster);

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
		 * Add all the points to their clusters
		 * and recalculate each cluster's centroid.
		 */

		#pragma omp parallel
		{
			// Clear all existing clusters.
			#pragma omp for
			for (auto& cluster : _clusters)
				cluster.clear();

			// Add each point to the cluster it belongs.
			#pragma omp for
			for (const point_t& point : _points)
				// Cluster's index is its id - 1.
				_clusters[point.cluster()->id() - 1].add_point(&point);

			// Recenter clusters because they contain new points.
			#pragma omp for
			for (auto& cluster : _clusters)
				cluster.recenter();
		}
	}
}

void kmeans_t::print_clusters(ostream& outstream, string indent, bool print_points) const
{
	for (const cluster_t& cluster : _clusters)
		cluster.print(outstream, indent, print_points);
}

void kmeans_t::print_points(ostream& outstream, string indent) const
{
	for (const point_t& point : _points)
		point.print(outstream, indent);
}
