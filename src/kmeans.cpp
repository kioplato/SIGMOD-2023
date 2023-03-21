#include <cstdint>
#include <vector>
#include <algorithm>
#include <queue>
#include <omp.h>
#include "kmeans.hpp"
#include "typedefs.hpp"

using namespace std;

/*
 * @brief Initialize K-Means algorithm with the number of
 * clusters to create and the number of iterations to perform.
 *
 * @param n_clusters The number of clusters to create.
 * @param n_iters The maximum number of iterations to perform.
 */
kmeans_t::kmeans_t(uint32_t n_clusters, uint32_t n_iters, points_t& points, uint32_t n_nearest_clusters)
: _n_clusters(n_clusters), _n_iters(n_iters), _points(points), _n_nearest_clusters(n_nearest_clusters)
{
	// Empty.
}

/*
 * @brief Find the nearest cluster to the specified point @assortee.
 *
 * @param clusters The clusters to search and find the nearest to @assortee.
 * @param assortee The point to find its nearest clusters.
 *
 * @return The address of the @assortee's nearest cluster.
 */
static inline cluster_t*
_find_nearest_cluster(clusters_t& clusters, const point_t& assortee)
{
	cluster_t* nearest_cluster = &clusters.front();
	double nearest_distance = euclidean_distance_aprox(nearest_cluster->centroid(), assortee);

	for (cluster_t& cluster : clusters) {
		double distance = euclidean_distance_aprox(cluster.centroid(), assortee);

		if (distance < nearest_distance) {
			nearest_cluster = &cluster;
			nearest_distance = distance;
		}
	}

	return nearest_cluster;
}

/*
 * @brief Find the m nearest clusters to @assortee.
 *
 * @param clusters The clusters to filter the m closest to @assortee.
 * @param assortee The point to find its m nearest clusters from @clusters.
 * @param m The number of nearest clusters to search into.
 *
 * @return The memory address of the @assortee's m nearest clusters,
 * sorted from the furthest to the nearest cluster.
 */
static inline const cluster_ptrs_t
_find_m_nearest_clusters(clusters_t& clusters, const point_t& assortee,
		const uint32_t& n_nearest_clusters)
{
	// Store the m nearest clusters here.
	cluster_ptrs_t nearest_clusters_vector;

	priority_queue<pair<double, cluster_t*>> nearest_clusters;

	for (cluster_t& cluster : clusters) {
		double distance = euclidean_distance_aprox(cluster.centroid(), assortee);

		if (nearest_clusters.size() < n_nearest_clusters)
			nearest_clusters.push({distance, &cluster});
		else if (nearest_clusters.top().first > distance) {
			nearest_clusters.pop();
			nearest_clusters.push({distance, &cluster});
		}
	}

	while (!nearest_clusters.empty()) {
		nearest_clusters_vector.push_back(nearest_clusters.top().second);
		nearest_clusters.pop();
	}

	return nearest_clusters_vector;
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
#ifdef VERBOSE
	cout << "Performing K-Means clustering." << endl;
#endif

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
		_points[index].clusters({&_clusters.back()});

		// Don't use the same point .
		used_points.push_back(index + 1);

		++c_cluster;
	}

#ifdef VERBOSE
	cout << "Created " << _clusters.size() << " clusters." << endl;
	cout << "Initialized clusters with these point IDs:" << endl;
	for (uint32_t id : used_points)
		cout << "\t" << id << endl;
	print_clusters(cout, "", true, false);
#endif

#ifdef VERBOSE
		cout << "K-Means 0 iteration." << endl;
#endif
	/*
	 * Initialize each point's nearest clusters. This initialization saves
	 * us from checking if the point has been initialized inside the loop.
	 * In essense we perform one K-Means iteration. This means we need to
	 * start from the 2nd iteration in the improvement loop below.
	 */
	#pragma omp parallel
	{
		// Find each point's n nearest clusters.
		#pragma omp for
		for (point_t& point : _points)
			point.clusters({_find_nearest_cluster(_clusters, point)});

		// Add each point to the cluster it belongs.
		#pragma omp for
		for (const point_t& point : _points)
			point.clusters().back()->add_point(&point);

		// Recenter clusters because they contain new points.
		#pragma omp for
		for (auto& cluster : _clusters)
			cluster.recenter();
	}

	/*
	 * Start improving the clusters and update each point's clusters.
	 *
	 * We either stop if we can no longer improve the clusters,
	 * or if maximum iterations @_n_iters have been performed.
	 */

	for (uint32_t c_iter = 1; c_iter < _n_iters; ++c_iter)
	{
#ifdef VERBOSE
		cout << "K-Means " << c_iter << " iteration." << endl;
#endif

		// We stop when we can no longer improve any cluster.
		bool done = true;

		// Add all points to their nearest cluster.
		#pragma omp parallel for reduction(&&: done)
		for (point_t& point : _points) {
			cluster_t* curr_cluster = point.clusters().front();
			cluster_t* best_cluster = _find_nearest_cluster(_clusters, point);

			if (curr_cluster != best_cluster) {
				done = false;
				point.clusters({best_cluster});
			}

			// TODO: Remove point from @curr_cluster_id and add it
			// to @best_cluster_id. A race could occur where two
			// points need to be removed from the same cluster or
			// added to the same cluster.
		}

		// If no cluster was improved then we are done.
		if (done) break;

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
				point.clusters().back()->add_point(&point);

			// Recenter clusters because they contain new points.
			#pragma omp for
			for (auto& cluster : _clusters)
				cluster.recenter();
		}
	}

	// Compute the @n_nearest_clusters of each point.
	#pragma omp parallel for
	for (point_t& point : _points)
		point.clusters(_find_m_nearest_clusters(_clusters, point, _n_nearest_clusters));
}

void kmeans_t::print_clusters(ostream& outstream, string indent, bool print_points, bool print_coords) const
{
	for (const cluster_t& cluster : _clusters)
		cluster.print(outstream, indent, print_points, print_coords);
}

void kmeans_t::print_points(ostream& outstream, string indent, bool print_coords) const
{
	for (const point_t& point : _points)
		point.print(outstream, indent, print_coords);
}
