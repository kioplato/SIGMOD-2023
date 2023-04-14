#include <iostream>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <omp.h>

#include "heap.hpp"

using namespace std;

/**
 * @brief Print provided arguments to stderr stream and exit with error code 1.
 * Prefix the provided arguments with the program's name and fatal.
 * Use this function to describe an error that is expected.
 *
 * @param args_t The provided arguments to print before exiting.
 *
 * @return The program exits with with error code 1.
 */
template<class... args_t>
inline void die(args_t... args)
{
	cerr << "[knng-clusters] fatal: ";
	(cerr << ... << args) << endl;

	exit(1);
}

/**
 * @brief Print provided arguments to stderr stream and exit with error code 1.
 * Prefix the provided arguments with the program's name and internal.
 * Use this function to describe an error that should not occur.
 *
 * @param args_t The provided arguments to print before exiting.
 *
 * @return The program exits with with error code 1.
 */
template<class... args_t>
inline void diei(args_t... args)
{
	cerr << "[knng-clusters] internal: ";
	(cerr << ... << args) << endl;

	exit(1);
}

struct Point {
	// Identifies the point uniquely.
	uint32_t id;

	// The point's coordinates.
	float coordinates[100];

	// Synchronization lock for TD phase.
	omp_lock_t lock;
	// The current number of neighbors.
	uint32_t n_neighbors;
	// The near neighbors to this point.
	Pair neighbors[100];
};

/**
 * @brief Compute the euclidean distance of two points.
 *
 * @param point1 The first point to use for euclidean distance.
 * @param point2 The second point to use for euclidean distance.
 *
 * @return The euclidean distance between @point1 and @point2.
 */
static inline double
euclidean_distance_aprox(const Point& point1, const Point& point2)
{
	double distance = 0;

	for (uint32_t c_dim = 0; c_dim < 100; ++c_dim)
		distance += (point1.coordinates[c_dim] - point2.coordinates[c_dim]) * (point1.coordinates[c_dim] - point2.coordinates[c_dim]);

	return distance;
}

pair<Point*, uint32_t> read_dataset(const string& path)
{
	// Open the dataset from the disk.
	ifstream ifs(path, ios::binary);
	if (!ifs.is_open())
		die("failed to open dataset at ", path);

	// Read number of points in the dataset.
	uint32_t dataset_cardinality;
	ifs.read((char*)&dataset_cardinality, sizeof(uint32_t));

	cout << "Dataset's cardinality is " << dataset_cardinality << endl;

	// Reserve memory for the points.
	// In the paper a point is the same as SajObj in Table II.
	Point *points = (Point*)malloc(sizeof(Point) * dataset_cardinality);

	// The id of the next point we are going to read.
	uint32_t next_point_id = 0;

	// Read one by one the points of the dataset.
	while (ifs.read((char*)&(points[next_point_id].coordinates), 100 * sizeof(float))) {
		points[next_point_id].id = next_point_id;
		points[next_point_id].n_neighbors = 0;
		omp_init_lock(&points[next_point_id].lock);

		++next_point_id;
	}

	ifs.close();

	return {points, dataset_cardinality};
}

void kmeans(uint32_t BU_indices[], const uint32_t& part_begin, const uint32_t& part_size, const uint32_t& n_clusters, float *distances, const uint32_t& next_BU_indices_start, unordered_map<uint32_t,vector<uint32_t>>& representatives)
{
	printf("[thread %u]: In K-means.\n", omp_get_thread_num());

	// The medoid of each cluster, with indices in [0, part_size).
	uint32_t medoid_raw[n_clusters];
	// The distances between the members of each cluster.
	float medoid_distances[n_clusters][part_size];
	// The members of each cluster.
	uint32_t members[n_clusters][part_size];
	// The members of each cluster with indices in [0, part_size).
	uint32_t members_raw[n_clusters][part_size];
	// How many members each cluster has.
	uint32_t n_members[n_clusters];
	// Initialize the size of each cluster to zero.
	memset(n_members, 0, sizeof(uint32_t) * n_clusters);

	// Initialize the clusters with the first @n_clusters points.
	for (uint32_t c_cluster = 0; c_cluster < n_clusters; ++c_cluster) {
		medoid_raw[c_cluster] = c_cluster;//BU_indices[part_begin + c_cluster];

		medoid_distances[c_cluster][0] = 0;

		// The centroid is a member of the cluster.
		members[c_cluster][0] = BU_indices[part_begin + c_cluster];
		members_raw[c_cluster][0] = c_cluster;

		++n_members[c_cluster];
	}

	// Add each point to its closest centroid.
	for (uint32_t c_point = n_clusters; c_point < part_size; ++c_point) {
		// Initialize the point's best cluster as the first cluster.
		uint32_t best_cluster_index = 0;
		float best_distance = distances[c_point * part_size + medoid_raw[0]];

		// Iterate over all the other clusters and find the best cluster.
		for (uint32_t c_cluster = 1; c_cluster < n_clusters; ++c_cluster) {
			if (distances[c_point * part_size + medoid_raw[c_cluster]] < best_distance) {
				best_cluster_index = c_cluster;
				best_distance = distances[c_point * part_size + medoid_raw[c_cluster]];
			}
		}

		// Add the point to the best cluster we found.
		members[best_cluster_index][n_members[best_cluster_index]] = BU_indices[part_begin + c_point];
		members_raw[best_cluster_index][n_members[best_cluster_index]] = c_point;

		// Update the medoid distances and the cluster's medoid.
		float distance = distances[members_raw[best_cluster_index][0] * part_size + c_point];
		medoid_distances[best_cluster_index][0] += distance;
		medoid_distances[best_cluster_index][n_members[best_cluster_index]] = distance;

		uint32_t best_medoid_index = 0;
		float best_medoid_distance = medoid_distances[best_cluster_index][0];

		for (uint32_t c_member = 1; c_member < n_members[best_cluster_index]; ++c_member) {
			distance = distances[members_raw[best_cluster_index][c_member] * part_size + c_point];
			medoid_distances[best_cluster_index][c_member] += distance;
			medoid_distances[best_cluster_index][n_members[best_cluster_index]] += distance;

			printf("[thread %u]: medoid_distances[best_cluster_index][c_member] = %f\n", omp_get_thread_num(), medoid_distances[best_cluster_index][c_member]);
			printf("[thread %u]: best_medoid_distance = %f\n", omp_get_thread_num(), best_medoid_distance);

			// Update the current best medoid.
			if (medoid_distances[best_cluster_index][c_member] < best_medoid_distance) {
				best_medoid_index = c_member;
				best_medoid_distance = medoid_distances[best_cluster_index][c_member];
			}
		}
		// Compare with the point we just inserted.
		if (medoid_distances[best_cluster_index][n_members[best_cluster_index]] < best_medoid_distance) {
				best_medoid_index = n_members[best_cluster_index];
				best_medoid_distance = medoid_distances[best_cluster_index][n_members[best_cluster_index]];
		}

		medoid_raw[best_cluster_index] = best_medoid_index;

		++n_members[best_cluster_index];
	}

	// The actual medoid indices in @points.
	uint32_t medoid[n_clusters];

	// Add the medoids to the hash table with the representatives.
	// While at it, also update the next BU level's indices.
	for (uint32_t c_cluster = 0; c_cluster < n_clusters; ++c_cluster) {
		representatives[BU_indices[part_begin + medoid_raw[c_cluster]]] = vector<uint32_t>(members[c_cluster], members[c_cluster] + n_members[c_cluster]);
		medoid[c_cluster] = BU_indices[part_begin + medoid_raw[c_cluster]];
	}

	// Copy the next BU level indices to @BU_indices.
	memcpy(BU_indices + next_BU_indices_start, medoid, sizeof(uint32_t) * n_clusters);

	printf("[thread %u]: Out K-means.\n", omp_get_thread_num());
}

static inline void
compute_distances(Point *points, uint32_t BU_indices[], const uint32_t& thr_part_begin,
		const uint32_t& part_size, float *distances)
{
	printf("[thread %u]: In compute_distances.\n", omp_get_thread_num());

	Pair pair;

	for (uint32_t c_diag = 0; c_diag < part_size; ++c_diag)
		distances[c_diag * part_size + c_diag] = 0;

	for (uint32_t c_row = 0; c_row < part_size; ++c_row) {
		for (uint32_t c_col = c_row + 1; c_col < part_size; ++c_col) {
			printf("[thread %u]: Iterating distances.\n", omp_get_thread_num());
			printf("[thread %u]: c_row = %u.\n", omp_get_thread_num(), c_row);
			printf("[thread %u]: c_col = %u.\n", omp_get_thread_num(), c_col);
			Point& from = points[BU_indices[thr_part_begin + c_row]];
			Point& to = points[BU_indices[thr_part_begin + c_col]];

			float distance = euclidean_distance_aprox(from, to);

			distances[c_row * part_size + c_col] = distance;
			distances[c_col * part_size + c_row] = distance;

			// Setup the pair for the @from point.
			pair = {from.id, to.id, distance};

			if (from.n_neighbors < 100)
				heap_insert(from.neighbors, from.n_neighbors, pair);
			else if (from.neighbors[0].distance > distance) {
				from.neighbors[0] = pair;
				heapify_subroot(from.neighbors, from.n_neighbors, 0);
			}

			// Setup the pair for the @from point.
			pair = {to.id, from.id, distance};

			if (to.n_neighbors < 100)
				heap_insert(to.neighbors, to.n_neighbors, pair);
			else if (to.neighbors[0].distance > distance) {
				to.neighbors[0] = pair;
				heapify_subroot(to.neighbors, to.n_neighbors, 0);
			}
		}
	}

	printf("[thread %u]: Out compute_distances.\n", omp_get_thread_num());
}

void near_neighbor_join(Point *points, uint32_t n_points, uint32_t partition_size, uint32_t n_clusters)
{
	#pragma omp parallel
	{
		// The number of total threads.
		const uint32_t n_thr = omp_get_num_threads();
		// The ID of this thread.
		const uint32_t thr_id = omp_get_thread_num();

		#pragma omp master
		printf("[thread %u]: Using %u threads.\n", thr_id, n_thr);

		/* Compute the thread's chunk info. */
		// Each thread's base chunk size.
		uint32_t base_chunk_size = n_points / n_thr;
		// How many threads receive one more point.
		uint32_t chunk_one_more = n_points % n_thr;
		// This thread's chunk size.
		uint32_t thr_chunk_size = base_chunk_size + (chunk_one_more > thr_id);

		// The thread's chunk start index.
		uint32_t thr_index_begin = base_chunk_size * thr_id + min(thr_id, chunk_one_more);
		// The thread's chunk end index (non-inclusive).
		uint32_t thr_index_end = thr_index_begin + thr_chunk_size;

		// The size of the next BU level of this thread.
		uint32_t thr_BU_size = thr_chunk_size;

		printf("[thread %u]: Chunk size = %u.\n", thr_id, thr_chunk_size);

		/* Compute the thread's partitions' info. */
		// The thread's number of partitions.
		uint32_t thr_n_parts = ceil((float)thr_BU_size / partition_size);
		// The size of each partition.
		uint32_t thr_base_part_size = thr_BU_size / thr_n_parts;
		// How many of this thread's paritions receive one more point.
		uint32_t thr_part_one_more = thr_BU_size % thr_n_parts;

		// Buffer to store each BU level's points' indices.
		uint32_t BU_indices[thr_chunk_size];
		// Initialize the buffer with the first BU level's indices.
		for (uint32_t i = 0; i < thr_chunk_size; ++i)
			BU_indices[i] = thr_index_begin + i;

		// The size of the next BU level.
		uint32_t thr_next_BU_size = thr_n_parts * n_clusters;

		// Which points each representative represents.
		vector<unordered_map<uint32_t, vector<uint32_t>>> representatives;

		// We are done when the points of the next BU iteration
		// are less then the specified @partition_size.
		bool thr_done = thr_BU_size < partition_size;

		printf("[thread %u]: Done = %d.\n", thr_id, thr_done);

		// From which index to write the next BU level's indices.
		uint32_t next_BU_indices_start = 0;

		// Where the next thread's partition begin.
		uint32_t thr_part_begin;
		// The size of the current partition.
		uint32_t part_size;

		uint32_t current_iteration = 1;

		while (!thr_done) {
			printf("[thread %u]: Performing %u-th iteration.\n", thr_id, current_iteration);
			printf("[thread %u]: Partitions in this iteration = %u.\n", thr_id, thr_n_parts);

			representatives.push_back(unordered_map<uint32_t,vector<uint32_t>>());

			thr_part_begin = 0;

			// Iterate over the partitions of this BU level.
			for (uint32_t c_part = 0; c_part < thr_n_parts; ++c_part) {
				// The size of this partition.
				part_size = thr_base_part_size + (c_part < thr_part_one_more);

				printf("[thread %u]: Size of %u partition = %u.\n", thr_id, c_part, part_size);

				// Compute the distance between all pairs of the partition.
				float distances[part_size][part_size];
				compute_distances(points, BU_indices, thr_part_begin, part_size, (float*)distances);

				printf("[thread %u]: Printing distances:.\n", thr_id);
				for (uint32_t i = 0; i < part_size; ++i)
					for (uint32_t j = 0; j < part_size; ++j)
						printf("[thread %u]: distances[%u][%u] = %f\n", thr_id, i, j, distances[i][j]);

				// Perform K-Means clustering (one iteration).
				kmeans(BU_indices, thr_part_begin, part_size, n_clusters, (float*)distances, next_BU_indices_start, representatives.back());

				// Next partition start after the current one.
				thr_part_begin += thr_base_part_size + (c_part < thr_part_one_more);
				// Continue to write the next BU level's indices from where this iteration left off.
				next_BU_indices_start += n_clusters;

				// Printing the representatives.
				for (const unordered_map<uint32_t, vector<uint32_t>>& BU_level : representatives) {
					cout << "BU level." << endl;
					for (const auto& pair : BU_level) {
						cout << '{' << pair.first << ": ";

						for (const auto& item : pair.second)
							cout << item << ' ';

						cout << '}' << endl;
					}
				}

			}

			/* Bookkeeping for the next BU iteration level. */
			thr_BU_size = thr_n_parts * n_clusters;
			thr_done = thr_BU_size < partition_size;

			thr_n_parts = ceil((float)thr_BU_size / partition_size);
			thr_base_part_size = thr_BU_size / thr_n_parts;
			thr_part_one_more = thr_BU_size % thr_n_parts;

			next_BU_indices_start = 0;

			++current_iteration;
		}
	}
}

//void bottom_up(Point *points, uint32_t n_points, uint32_t k,
//		uint32_t partition_size, uint32_t n_clusters)
//{
//	// The total number of BU levels we will create.
//	uint32_t n_BU_levels = log2(n_points)/log2(partition_size/n_clusters);
//	// The starting index of each BU level.
//	uint32_t BU_levels[n_BU_levels];
//	// Set the 0-th BU level's starting index.
//	BU_levels[0] = 0;
//
//	// Where the current BU level's points start and end.
//	uint32_t index_start = 0;
//	uint32_t index_end = n_points - 1;
//
//	// The BU level's remaining points. First BU level is @n_points.
//	uint32_t BU_size = n_points;
//
//	// We are done when the partition size is smaller than @part_size.
//	bool done;
//
//	/*
//	 * Store the most bookkeeping variables to thread local stack.
//	 * This way less cache synchronization will be performed, hopefully.
//	 */
//	#pragma omp parallel
//	{
//		// The number of total threads.
//		const uint32_t n_thr = omp_get_num_threads();
//		// The ID of this thread.
//		const uint32_t thr_id = omp_get_thread_num();
//
//		// We are done when the remaining points for
//		// each thread are less than the partition size.
//		#pragma omp single
//		done = (n_points / n_thr) < partition_size;
//
//		/* Compute the thread's chunk related info. */
//		// Each thread's base chunk size.
//		uint32_t base_chunk_size = BU_size / n_thr;
//		// How many threads receive one more point.
//		uint32_t chunk_one_more = BU_size % n_thr;
//		// The thread's chunk size for this BU level.
//		uint32_t thr_chunk_size = base_chunk_size + (chunk_one_more > thr_id);
//		// The thread's chunk start index in @points.
//		uint32_t thr_index_start = index_start + base_chunk_size * thr_id + min(thr_id, chunk_one_more);
//
//		/* Compute the thread's partitions' related info. */
//		// The thread's number of partitions.
//		uint32_t thr_n_parts = ceil((BU_size / n_thr) / partition_size);
//		// The size of each partition of the thread's chunk.
//		uint32_t thr_base_part_size = thr_chunk_size / thr_n_parts;
//		// The first @thr_part_one_more partitions receive one more item.
//		uint32_t thr_part_one_more = thr_chunk_size % thr_n_parts;
//		// Where the next partition will start.
//		uint32_t thr_part_start = thr_index_start;
//
//		// The current level we are working on.
//		uint32_t c_BU_level = 0;
//
//		while (!done)
//		{
//			// The indices of the thread's centroids of one BU iteration.
//			uint32_t thr_centroids[thr_n_parts * n_clusters];
//
//			// Iterate over the thread's partitions.
//			for (uint32_t c_part = 0; c_part < thr_n_parts; ++c_part) {
//				// This partition's size.
//				const uint32_t part_size = thr_base_part_size + (c_part < thr_part_one_more);
//
//				// Compute the distance between all pairs of the partition.
//				// At the same time compute the initial nearest neighbors.
//				float distances[part_size][part_size];
//				compute_distances(points, thr_part_start, part_size, distances);
//
//				// Perform kmeans clustering with one iteration.
//				// At the same time each point gets assigned a representative.
//				kmeans(points, thr_part_start, part_size, thr_centroids, c_part, n_clusters, 1);
//
//				// Bookkeeping regarding next partition.
//				thr_part_start = part_start + part_size;
//			}
//
//			// Wait for everyone to stop reading their points.
//			// We want to move the representatives at the end.
//			#pragma omp barrier
//
//			// Write the centroids at the end of the buffer @points.
//			// This messes up the last points and that's why we barrier'ed.
//			memmove_centroids(points, thr_centroids, thr_n_parts * n_clusters);
//
//			/* Update shared bookkeeping variables. */
//			#pragma omp single
//			{
//				// Next BU level has different size then the current.
//				BU_size = n_thr * thr_n_parts * n_clusters;
//
//				// Check if we are done.
//				// We are when the partition size of each thread
//				// is smaller then the requested @partition_size.
//				done = (BU_size / n_thr) < partition_size;
//
//				// Next BU level starts at a new index.
//				// The end index remains the same.
//				index_start = n_points - BU_size;
//
//				// Add the starting index of the next level.
//				BU_levels[++c_BU_level] = index_start;
//			}
//
//			/* Update private bookkeeping variables. */
//			base_chunk_size = BU_size / n_thr;
//			chunk_one_more = BU_size % n_thr;
//			thr_chunk_size = base_chunk_size + (chunk_one_more > thr_id);
//			thr_index_start = index_start + base_chunk_size * thr_id + min(thr_id, chunk_one_more);
//
//			thr_n_parts = ceil((BU_size / n_thr) / partition_size);
//			thr_base_part_size = thr_chunk_size / thr_n_parts;
//			thr_part_one_more = thr_chunk_size % thr_n_parts;
//			thr_part_start = thr_index_start;
//		}
//}

//void top_down(Point *points, uint32_t n_points, uint32_t *BU_indices, uint32_t BU_n_levels, uint32_t P)
//{
//	// Find the P best pairs from the last BU level.
//	Pair *topP = find_topP(points, n_points, BU_indices[BU_n_levels - 1], P);
//}

void write_knng(const Point *points, const uint32_t& n_points, const string& path)
{
	ofstream ofs(path, ios::out | ios::binary);

	for (uint32_t c_point = 0; c_point < n_points; ++c_point)
		for (uint32_t c_near = 0; c_near < 100; ++c_near)
			ofs.write((char*)&points[c_point].neighbors[c_near].to_id, sizeof(uint32_t));

	ofs.close();
}

int main(void)
{
	// Don't use dynamic number of threads according to system load.
	omp_set_dynamic(0);
	// Don't use nested parallelism.
	omp_set_nested(0);
	// The number of threads to create will match to the number of cores.
	omp_set_num_threads(omp_get_num_procs());

	/* The hyperparameters of the problem (Table I). */
	// The size of each partition per thread.
	uint32_t partition_size = 3;
	// The number of clusters to create per partition.
	uint32_t n_clusters = 1;
	// Number of top object pairs for each TD iteration.
	uint32_t P = 5;

	printf("Hyperparameters selected:\n");
	printf("\tMinimum partition size = %u\n", partition_size);
	printf("\tClusters to create per partition = %u\n", n_clusters);
	printf("\tTopP pair to check = %u\n", P);

	// Read the dataset from the dummy-data.txt file.
	// The returned memory should be free'd.
	auto [points, n_points] = read_dataset("../../datasets/50.bin");

	cout << "Printing dataset:" << endl;
	for (uint32_t c_point = 0; c_point < n_points; ++c_point)
		for (uint32_t c_dim = 0; c_dim < 100; ++c_dim)
			cout << c_point << "-th point's " << c_dim << "-th dimension = " << points[c_point].coordinates[c_dim] << endl;

	// Perform near neighbor join algorithm.
	printf("===== Near Neighbor Join (start) =====\n");
	near_neighbor_join(points, n_points, partition_size, n_clusters);
	printf("===== Near Neighbor Join (exits) =====\n");

	for (uint32_t c_point = 0; c_point < n_points; ++c_point)
		cout << c_point << "-th point has " << points[c_point].n_neighbors << " neighbors." << endl;

	// Write the knng to disk.
	//write_knng(points, n_points, "output.bin");

	free(points);

	return EXIT_SUCCESS;
}
