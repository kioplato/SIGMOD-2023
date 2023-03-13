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

	/*
	 * Initialize the @nearest_neighbors of @point by adding a
	 * point from its cluster, but be careful, ignore itself.
	 */

	// The @point's cluster.
	//if (point.cluster() == NULL) {
	//	cout << "fatal internal: found a point with null cluster." << endl;
	//	point.print(cout, "");

	//	exit(1);
	//}

	const vector<point_t>& candidates = point.cluster()->points();

	size_t c_cand = 0;

	for (; c_cand < candidates.size(); ++c_cand) {
		// Skip itself. A point isn't a neighbor of itself.
		if (candidates[c_cand].id() == point.id())
			continue;

		const point_t& candidate = candidates[c_cand];
		double distance = euclidean_distance_aprox(point, candidate);

		if (nearest_neighbors.size() < k)
			nearest_neighbors.push({distance, candidate.id() - 1});
		else if (nearest_neighbors.top().first > distance) {
			nearest_neighbors.pop();
			nearest_neighbors.push({distance, candidate.id() - 1});
		}
	}

	//if (nearest_neighbors.size() < 100) {
	//	cerr << "fatal internal: point without 100 nn." << endl;
	//	exit(1);
	//}

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

vector<vector<uint32_t>> create_knng(vector<point_t>& points, uint32_t k)
{
	/*
	 * Run K-Means clustering. Using this method we exhaustively search
	 * for the k nearest neighbors of a point in the cluster it belongs.
	 */
	//cout << "In create_knng: Running K-Means." << endl;
	kmeans_t kmeans(2, 100, points);
	kmeans.run();
	//cout << "In create_knng: Done K-Means." << endl;

	//cout << "In create_knng: Printing the clustering result." << endl;
	//kmeans.print_clusters(cout, "");

	//cout << "Printing any points with NULL cluster ptr." << endl;
	//for (const point_t& point : points) {
	//	if (point.cluster() == NULL)
	//		point.print(cout, "");
	//}

	//cout << "Creating the knng." << endl;
	vector<vector<uint32_t>> knng;
	knng.resize(points.size());

	#pragma omp parallel for
	for (size_t c_point = 0; c_point < points.size(); ++c_point) {
		//printf("Calculating k-nn of %zu-th point.\n", c_point);
		knng[c_point] = knn_of_point(points[c_point], k);
	}

	return knng;
}
