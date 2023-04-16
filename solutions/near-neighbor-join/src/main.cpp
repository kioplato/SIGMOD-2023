#include <iostream>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <unistd.h>
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

static inline uint32_t triu(uint32_t i, uint32_t j, uint32_t size)
{
	return i*(size-1) - ((i-1)*i)/2 + j - i - 1;
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
	// The actual medoid indices in @points.
	uint32_t medoid[n_clusters];
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

		// The medoid is a member of the cluster.
		members[c_cluster][0] = BU_indices[part_begin + c_cluster];
		members_raw[c_cluster][0] = c_cluster;

		++n_members[c_cluster];
	}

	// Add each point to its closest centroid.
	for (uint32_t c_point = n_clusters; c_point < part_size; ++c_point) {
		// Initialize the point's best cluster as the first cluster.
		uint32_t best_cluster_index = 0;
		// medoid_raw[0] is less than c_point always, since medoid_raw[0] is before c_point.
		float best_distance = distances[triu(medoid_raw[0], c_point, part_size)];

		// Iterate over all the other clusters and find the best cluster.
		for (uint32_t c_cluster = 1; c_cluster < n_clusters; ++c_cluster) {
			if (distances[triu(medoid_raw[c_cluster], c_point, part_size)] < best_distance) {
				best_cluster_index = c_cluster;
				best_distance = distances[triu(medoid_raw[c_cluster], c_point, part_size)];
			}
		}

		// Add the point to the best cluster we found.
		members[best_cluster_index][n_members[best_cluster_index]] = BU_indices[part_begin + c_point];
		members_raw[best_cluster_index][n_members[best_cluster_index]] = c_point;

		// Update the medoid distances and the cluster's medoid for more accuracy.
		float distance = distances[triu(members_raw[best_cluster_index][0], c_point, part_size)];
		medoid_distances[best_cluster_index][0] += distance;
		medoid_distances[best_cluster_index][n_members[best_cluster_index]] = distance;

		uint32_t best_medoid_index = 0;
		float best_medoid_distance = medoid_distances[best_cluster_index][0];

		for (uint32_t c_member = 1; c_member < n_members[best_cluster_index]; ++c_member) {
			distance = distances[triu(members_raw[best_cluster_index][c_member], c_point, part_size)];
			medoid_distances[best_cluster_index][c_member] += distance;
			medoid_distances[best_cluster_index][n_members[best_cluster_index]] += distance;

			//printf("[thread %u]: medoid_distances[best_cluster_index][c_member] = %f\n", omp_get_thread_num(), medoid_distances[best_cluster_index][c_member]);
			//printf("[thread %u]: best_medoid_distance = %f\n", omp_get_thread_num(), best_medoid_distance);

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

		// The index of the medoid of the @c_cluster in @members.
		medoid_raw[best_cluster_index] = members_raw[best_cluster_index][best_medoid_index];

		++n_members[best_cluster_index];
	}

	// Add the medoids to the hash table with the representatives.
	// While at it, also populate the @medoid array with the true indices for the next BU level.
	for (uint32_t c_cluster = 0; c_cluster < n_clusters; ++c_cluster) {
		representatives[BU_indices[part_begin + medoid_raw[c_cluster]]] = vector<uint32_t>(members[c_cluster], members[c_cluster] + n_members[c_cluster]);
		medoid[c_cluster] = BU_indices[part_begin + medoid_raw[c_cluster]];
	}

	// Copy the next BU level indices to @BU_indices.
	memcpy(BU_indices + next_BU_indices_start, medoid, sizeof(uint32_t) * n_clusters);

	//printf("[thread %u]: Printing the medoids of this partition:\n", omp_get_thread_num());
	//for (uint32_t i = 0; i < n_clusters; ++i) {
	//	printf("[thread %u]: medoid_raw[%u] = %u.\n", omp_get_thread_num(), i, medoid_raw[i]);
	//	printf("[thread %u]: medoid[%u] = %u.\n", omp_get_thread_num(), i, medoid[i]);
	//}

	//printf("[thread %u]: Out K-means.\n", omp_get_thread_num());
}

static inline void
compute_distances(Point *points, uint32_t BU_indices[], const uint32_t& thr_part_begin,
		const uint32_t& part_size, float *distances)
{
	//printf("[thread %u]: In compute_distances.\n", omp_get_thread_num());

	Pair pair;

	for (uint32_t c_row = 0; c_row < part_size; ++c_row) {
		for (uint32_t c_col = c_row + 1; c_col < part_size; ++c_col) {
			//printf("[thread %u]: Iterating distances.\n", omp_get_thread_num());
			//printf("[thread %u]: c_row = %u.\n", omp_get_thread_num(), c_row);
			//printf("[thread %u]: c_col = %u.\n", omp_get_thread_num(), c_col);
			Point& from = points[BU_indices[thr_part_begin + c_row]];
			Point& to = points[BU_indices[thr_part_begin + c_col]];

			float distance = euclidean_distance_aprox(from, to);

			distances[triu(c_row, c_col, part_size)] = distance;

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

	//printf("[thread %u]: Out compute_distances.\n", omp_get_thread_num());
}

void near_neighbor_join(Point *points, uint32_t n_points, uint32_t partition_size, uint32_t n_clusters, uint32_t P)
{
	/* Variables that are shared between all threads. */
	// Buffer to write the last BU level's representatives of all threads.
	uint32_t *last_BU_level_representatives = NULL;

	// Buffer to write the topP pairs of each TD level.
	Pair *topP_pairs = NULL;

	// The from and to candidates of the topP pairs. The first candidate of
	// each topP pair is the number of candidates stored in the from / to candidates.
	uint32_t *from_cands = NULL;
	uint32_t *to_cands = NULL;

	/* The near neighbor join algorithm starts. */
	#pragma omp parallel
	{
		// The number of total threads.
		const uint32_t n_thr = omp_get_num_threads();
		// The ID of this thread.
		const uint32_t thr_id = omp_get_thread_num();

		//#pragma omp single
		//printf("[thread %u]: Using %u threads.\n", thr_id, n_thr);

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

		//printf("[thread %u]: Chunk size = %u.\n", thr_id, thr_chunk_size);

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

		// Which points each representative represents.
		vector<unordered_map<uint32_t, vector<uint32_t>>> representatives;

		// We are done when the points of the next BU iteration
		// are less then the specified @partition_size.
		bool thr_done = thr_BU_size <= partition_size;
		//bool thr_done = thr_BU_size / (partition_size / n_clusters);

		//printf("[thread %u]: Done = %d.\n", thr_id, thr_done);

		// From which index to write the next BU level's indices.
		uint32_t next_BU_indices_start = 0;

		// Where the next thread's partition begin.
		uint32_t thr_part_begin;
		// The size of the current partition.
		uint32_t part_size = thr_base_part_size + (0 < thr_part_one_more);

		uint32_t current_iteration = 1;

		float *distances = (float*)malloc(sizeof(float) * ((part_size * part_size) - part_size) / 2);

		while (!thr_done) {
			//printf("[thread %u]: ========== Performing %u-th iteration.\n", thr_id, current_iteration);
			//printf("[thread %u]: Partitions in this iteration = %u.\n", thr_id, thr_n_parts);

			//printf("[thread %u]: Printing BU indices of the upcoming BU level.\n", thr_id);
			//for (uint32_t i = 0; i < thr_BU_size; ++i)
			//	printf("[thread %u]: BU_indices[%u] = %u\n", thr_id, i, BU_indices[i]);

			representatives.push_back(unordered_map<uint32_t,vector<uint32_t>>());

			thr_part_begin = 0;

			// Iterate over the partitions of this BU level.
			for (uint32_t c_part = 0; c_part < thr_n_parts; ++c_part) {
				// The size of this partition.
				part_size = thr_base_part_size + (c_part < thr_part_one_more);

				//printf("[thread %u]: Size of %u partition = %u.\n", thr_id, c_part, part_size);

				// Compute the distance between all pairs of the partition.
				//float distances[part_size][part_size];
				compute_distances(points, BU_indices, thr_part_begin, part_size, distances);

				//printf("[thread %u]: Printing distances:.\n", thr_id);
				//for (uint32_t i = 0; i < part_size; ++i)
					//for (uint32_t j = 0; j < part_size; ++j)
						//printf("[thread %u]: distances[%u][%u] = %f\n", thr_id, i, j, distances[i][j]);

				// Perform K-Means clustering (one iteration).
				kmeans(BU_indices, thr_part_begin, part_size, n_clusters, distances, next_BU_indices_start, representatives.back());

				// Next partition start after the current one.
				thr_part_begin += part_size;
				// Continue to write the next BU level's indices from where this iteration left off.
				next_BU_indices_start += n_clusters;
			}

			// Printing the representatives.
			//const unordered_map<uint32_t, vector<uint32_t>>& BU_level = representatives.back();
			//printf("[thread %u]: %u BU level:\n", thr_id, current_iteration);
			//for (const auto& pair : BU_level) {
			//	cout << '{' << pair.first << ": ";

			//	for (const auto& item : pair.second)
			//		cout << item << ' ';

			//	cout << '}' << endl;
			//}

			/* Bookkeeping for the next BU iteration level. */
			uint32_t thr_BU_size_prev = thr_BU_size;
			thr_BU_size = thr_n_parts * n_clusters;
			thr_done = thr_BU_size <= partition_size || thr_BU_size_prev == thr_BU_size;
			//thr_done = thr_BU_size / (partition_size / n_clusters);

			thr_n_parts = ceil((float)thr_BU_size / partition_size);
			thr_base_part_size = thr_BU_size / thr_n_parts;
			thr_part_one_more = thr_BU_size % thr_n_parts;

			next_BU_indices_start = 0;

			++current_iteration;
		}

		//#pragma omp critical
		//{
		//	printf("[thread %u]: Performed %u BU levels.\n", thr_id, current_iteration - 1);
		//	printf("[thread %u]: Representatives contain %lu levels.\n", thr_id, representatives.size());
		//	printf("[thread %u]: Printing how many representatives each BU level contains:\n", thr_id);
		//	uint32_t i = 0;
		//	for (const auto& BU_level : representatives)
		//		printf("[thread %u]: %u BU level contains %lu representatives.\n", thr_id, i++, BU_level.size());

		//	// Printing the representatives.
		//	const unordered_map<uint32_t, vector<uint32_t>>& BU_level = representatives.back();
		//	printf("[thread %u]: Printing last BU level.\n", thr_id);
		//	for (const auto& pair : BU_level) {
		//		cout << '{' << pair.first << ": ";

		//		for (const auto& item : pair.second)
		//			cout << item << ' ';

		//		cout << '}' << endl;
		//	}
		//}

		free(distances);

		/***** Top down phase. *****/
		const uint32_t n_topP = n_thr * P;
		// How many representatives we have at the last level. Each thread has the same number.
		const uint32_t n_representatives = representatives.back().size();
		// A thread reserves memory for the last BU level's representatives for all threads.
		#pragma omp single
		{
			//printf("[thread %u]: In single section reserving memory.\n", thr_id);
			last_BU_level_representatives = (uint32_t*)malloc(n_representatives * n_thr * sizeof(uint32_t));
			topP_pairs = (Pair*)malloc(n_thr * sizeof(Pair) * P);
			from_cands = (uint32_t*)malloc(n_topP * sizeof(uint32_t) * partition_size + n_topP * sizeof(uint32_t));
			to_cands = (uint32_t*)malloc(n_topP * sizeof(uint32_t) * partition_size + n_topP * sizeof(uint32_t));
		}
		//printf("[thread %u]: I have %u representatives at the last level.\n", thr_id, n_representatives);

		// Each thread writes its last BU level's representatives to @last_BU_level_representatives.
		uint32_t start_index = thr_id * n_representatives;
		auto iter = representatives.back().begin();
		for (uint32_t i = 0; i < n_representatives; ++i, ++iter)
			last_BU_level_representatives[start_index + i] = iter->first;

		// All threads must have written their representatives before proceeding.
		#pragma omp barrier

		//#pragma omp single
		//{
		//	printf("[thread %u]: Printing the last level's representatives.\n", thr_id);
		//	for (uint32_t i = 0; i < n_representatives * n_thr; ++i)
		//		printf("[thread %u]: last_BU_level_representatives[%u] = %u.\n", thr_id, i, last_BU_level_representatives[i]);
		//}

		// Each thread will take a weighted part and perform cross product.
		// The total number of representatives we need to weightedly split.
		const uint32_t total_representatives = n_thr * n_representatives;
		const uint32_t repr_chunk_base_size = total_representatives / (n_thr * 2);
		const uint32_t repr_chunk_one_more = total_representatives % (n_thr * 2);
		// The two chunks that belong to this thread.
		const uint32_t thr_head_chunk_index = thr_id * repr_chunk_base_size + min(thr_id, repr_chunk_one_more);
		const uint32_t thr_head_chunk_size = repr_chunk_base_size + (thr_id < repr_chunk_one_more);
		const uint32_t thr_tail_chunk_index = (n_thr * 2 - thr_id - 1) * repr_chunk_base_size + min(n_thr * 2 - thr_id - 1, repr_chunk_one_more);
		const uint32_t thr_tail_chunk_size = repr_chunk_base_size + (n_thr * 2 - thr_id - 1 < repr_chunk_one_more);

		//Pair cross_prod[thr_head_chunk_size * (total_representatives - 1)];

		// The starting index of the thread's topP.
		const uint32_t heap_start_index = thr_id * P;
		uint32_t heap_size = 0;
		// The thread's topP chunk. Treat it like a heap.
		Pair *topP_heap = &(topP_pairs[heap_start_index]);

		// Each thread knows the two chunks it owns. Perform the cross product.
		Pair pair;
		// Cross-product for the first chunk of the thread.
		for (uint32_t i = 0; i < thr_head_chunk_size; ++i) {
			for (uint32_t j = thr_head_chunk_index + i + 1; j < total_representatives; ++j) {
				float distance = euclidean_distance_aprox(points[last_BU_level_representatives[thr_head_chunk_index + i]], points[last_BU_level_representatives[j]]);
				pair = {points[last_BU_level_representatives[thr_head_chunk_index + i]].id, points[last_BU_level_representatives[j]].id, distance};

				//printf("[thread %u]: From first chunk: from_id = %u, to_id = %u.\n", thr_id, pair.from_id, pair.to_id);

				if (heap_size < P)
					heap_insert(topP_heap, heap_size, pair);
				else if (topP_heap[0].distance > distance) {
					topP_heap[0] = pair;
					heapify_subroot(topP_heap, heap_size, 0);
				}
			}
		}

		// Cross-product for the second chunk of thread.
		for (uint32_t i = 0; i < thr_tail_chunk_size; ++i) {
			for (uint32_t j = thr_tail_chunk_index + i + 1; j < total_representatives; ++j) {
				float distance = euclidean_distance_aprox(points[last_BU_level_representatives[thr_tail_chunk_index + i]], points[last_BU_level_representatives[j]]);
				pair = {points[last_BU_level_representatives[thr_tail_chunk_index + i]].id, points[last_BU_level_representatives[j]].id, distance};

				//printf("[thread %u]: From second chunk: from_id = %u, to_id = %u.\n", thr_id, pair.from_id, pair.to_id);

				if (heap_size < P)
					heap_insert(topP_heap, heap_size, pair);
				else if (topP_heap[0].distance > distance) {
					topP_heap[0] = pair;
					heapify_subroot(topP_heap, heap_size, 0);
				}
			}
		}

		#pragma omp barrier

		//#pragma omp critical
		//{
		//printf("[thread %u]: My topP pairs are:\n", thr_id);
		//for (uint32_t i = 0; i < P; ++i)
		//	printf("[thread %u]: from_id = %u, to_id = %u, distance = %f.\n", thr_id, topP_heap[i].from_id, topP_heap[i].to_id, topP_heap[i].distance);
		//}

		// Heap for preparing the next TD iteration's BU and TD pairs.
		// In the paper they are called "all_pairs".
		Pair thr_heap[P];
		uint32_t thr_heap_size = 0;
		auto riter = representatives.rbegin();

		while (riter != (representatives.rend() - 1)) {
			unordered_map<uint32_t, vector<uint32_t>>& curr_BU_level = *riter;

			// The topP pairs have been calculated. Time to populate their from and to candidates.
			uint32_t ids_to_erase[n_representatives * 2];
			uint32_t n_ids_to_erase = 0;

			for (uint32_t i = 0; i < n_topP; ++i) {
				if (curr_BU_level.count(topP_pairs[i].from_id)) {
					//printf("[thread %u]: Yes! I know about %u. It represents %lu points.\n", thr_id, topP_pairs[i].from_id, curr_BU_level[topP_pairs[i].from_id].size());

					memcpy(&from_cands[i * (partition_size + 1) + 1], curr_BU_level[topP_pairs[i].from_id].data(), sizeof(uint32_t) * curr_BU_level[topP_pairs[i].from_id].size());
					from_cands[i * (partition_size + 1)] = curr_BU_level[topP_pairs[i].from_id].size();

					ids_to_erase[n_ids_to_erase++] = curr_BU_level.count(topP_pairs[i].from_id);
				}
				if (curr_BU_level.count(topP_pairs[i].to_id)) {
					//printf("[thread %u]: Yes! I know about %u. It represents %lu points.\n", thr_id, topP_pairs[i].to_id, curr_BU_level[topP_pairs[i].to_id].size());

					memcpy(&to_cands[i * (partition_size + 1) + 1], curr_BU_level[topP_pairs[i].to_id].data(), sizeof(uint32_t) * curr_BU_level[topP_pairs[i].to_id].size());
					to_cands[i * (partition_size + 1)] = curr_BU_level[topP_pairs[i].to_id].size();

					ids_to_erase[n_ids_to_erase++] = curr_BU_level.count(topP_pairs[i].to_id);
				}
			}

			#pragma omp barrier

			for (uint32_t i = 0; i < n_ids_to_erase; ++i)
				curr_BU_level.erase(ids_to_erase[i]);

			// Print the from and to candidates of each topP pair.
			//#pragma omp master
			//{
			//	printf("[thread %u]: ================================================================.\n", thr_id);
			//	printf("[thread %u]: Printing the from candidates and to candidates of the topP pairs.\n", thr_id);
			//	for (uint32_t i = 0; i < 8 * P; ++i) {
			//		printf("[thread %u]: from_id = %u, to_id = %u, distance = %f.\n", thr_id, topP_pairs[i].from_id, topP_pairs[i].to_id, topP_pairs[i].distance);
			//		const uint32_t n_from_cands = from_cands[i * (partition_size + 1)];
			//		const uint32_t n_to_cands = to_cands[i * (partition_size + 1)];
			//		printf("[thread %u]: from_cands contain %u indices.\n", thr_id, n_from_cands);
			//		printf("[thread %u]: to_cands contain %u indices.\n", thr_id, n_to_cands);

			//		//printf("[thread %u]: Printing the from candidates:\n", thr_id);
			//		//for (uint32_t j = 0; j < n_from_cands; ++j)
			//		//	printf("[thread %u]: from_cands[%u] = %u.\n", thr_id, j, from_cands[heap_start_index + i * (partition_size + 1) + 1 + j]);

			//		//printf("[thread %u]: Printing the to candidates:\n", thr_id);
			//		//for (uint32_t j = 0; j < n_to_cands; ++j)
			//		//	printf("[thread %u]: to_cands[%u] = %u.\n", thr_id, j, to_cands[heap_start_index + i * (partition_size + 1) + 1 + j]);
			//	}
			//	printf("[thread %u]: ================================================================.\n", thr_id);
			//}
			//#pragma omp barrier

			thr_heap_size = 0;

			// Only representatives that don't exist in topP remain.
			for (auto iter : curr_BU_level) {
				for (uint32_t representee_index : iter.second) {
					for (uint32_t c_neigh = 0; c_neigh < points[representee_index].n_neighbors; ++c_neigh) {
						if (thr_heap_size < P)
							heap_insert(thr_heap, thr_heap_size, points[representee_index].neighbors[c_neigh]);
						if (points[representee_index].neighbors[c_neigh].distance < thr_heap[0].distance) {
							thr_heap[0] = points[representee_index].neighbors[c_neigh];
							heapify_subroot(thr_heap, thr_heap_size, 0);
						}
					}
				}
			}

			// Barrier?

			// Each thread will perform cross product between the from candidates and to candidates of the P topP pairs it calculated.
			// The cross product is also performed in from/to candidates seperately.

			for (uint32_t i = 0; i < P; ++i) {
				// Perform the all pairs cross product on from candidates.
				const uint32_t n_from_cands = from_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1)];
				const uint32_t n_to_cands = to_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1)];
				for (uint32_t i_cand = 0; i_cand < n_from_cands; ++i_cand) {
					for (uint32_t j_cand = i + 1; j_cand < n_from_cands; ++j_cand) {
						uint32_t from_index = from_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + i_cand];
						uint32_t to_index = from_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + j_cand];

						float distance = euclidean_distance_aprox(points[from_index], points[to_index]);

						pair = {from_index, to_index, distance};

						// Update next iteration's topP pairs calculated by this thread.
						if (thr_heap_size < P)
							heap_insert(thr_heap, thr_heap_size, pair);
						else if (thr_heap[0].distance > pair.distance) {
							thr_heap[0] = pair;
							heapify_subroot(thr_heap, thr_heap_size, 0);
						}

						// Lock the from point to perform update.
						omp_set_lock(&points[from_index].lock);
						if (points[from_index].n_neighbors < 100)
							heap_insert(points[from_index].neighbors, points[from_index].n_neighbors, pair);
						else if (points[from_index].neighbors[0].distance > distance) {
							points[from_index].neighbors[0] = pair;
							heapify_subroot(points[from_index].neighbors, points[from_index].n_neighbors, 0);
						}
						omp_unset_lock(&points[from_index].lock);

						// Lock the to point to perform update.
						pair = {to_index, from_index, distance};
						omp_set_lock(&points[to_index].lock);
						if (points[to_index].n_neighbors < 100)
							heap_insert(points[to_index].neighbors, points[to_index].n_neighbors, pair);
						else if (points[to_index].neighbors[0].distance > distance) {
							points[to_index].neighbors[0] = pair;
							heapify_subroot(points[to_index].neighbors, points[to_index].n_neighbors, 0);
						}
						omp_unset_lock(&points[to_index].lock);
					}
				}

				// Perform all pairs cross product on from-to candidates.
				for (uint32_t i_cand = 0; i_cand < n_from_cands; ++i_cand) {
					for (uint32_t j_cand = 0; j_cand < n_to_cands; ++j_cand) {
						uint32_t from_index = from_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + i_cand];
						uint32_t to_index = to_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + j_cand];

						float distance = euclidean_distance_aprox(points[from_index], points[to_index]);

						pair = {from_index, to_index, distance};

						// Update next iteration's topP pairs calculated by this thread.
						if (thr_heap_size < P)
							heap_insert(thr_heap, thr_heap_size, pair);
						else if (thr_heap[0].distance > pair.distance) {
							thr_heap[0] = pair;
							heapify_subroot(thr_heap, thr_heap_size, 0);
						}

						// Lock the from point to perform update.
						omp_set_lock(&points[from_index].lock);
						if (points[from_index].n_neighbors < 100)
							heap_insert(points[from_index].neighbors, points[from_index].n_neighbors, pair);
						else if (points[from_index].neighbors[0].distance > distance) {
							points[from_index].neighbors[0] = pair;
							heapify_subroot(points[from_index].neighbors, points[from_index].n_neighbors, 0);
						}
						omp_unset_lock(&points[from_index].lock);

						// Lock the to point to perform update.
						pair = {to_index, from_index, distance};
						omp_set_lock(&points[to_index].lock);
						if (points[to_index].n_neighbors < 100)
							heap_insert(points[to_index].neighbors, points[to_index].n_neighbors, pair);
						else if (points[to_index].neighbors[0].distance > distance) {
							points[to_index].neighbors[0] = pair;
							heapify_subroot(points[to_index].neighbors, points[to_index].n_neighbors, 0);
						}
						omp_unset_lock(&points[to_index].lock);
					}
				}

				// Perform all pairs cross product on to candidates.
				for (uint32_t i_cand = 0; i_cand < n_to_cands; ++i_cand) {
					for (uint32_t j_cand = i + 1; j_cand < n_to_cands; ++j_cand) {
						uint32_t from_index = to_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + i_cand];
						uint32_t to_index = to_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + j_cand];

						float distance = euclidean_distance_aprox(points[from_index], points[to_index]);

						pair = {from_index, to_index, distance};

						if (thr_heap_size < P)
							heap_insert(thr_heap, thr_heap_size, pair);
						else if (thr_heap[0].distance > pair.distance) {
							thr_heap[0] = pair;
							heapify_subroot(thr_heap, thr_heap_size, 0);
						}

						// Lock the from point to perform update.
						omp_set_lock(&points[from_index].lock);
						if (points[from_index].n_neighbors < 100)
							heap_insert(points[from_index].neighbors, points[from_index].n_neighbors, pair);
						else if (points[from_index].neighbors[0].distance > distance) {
							points[from_index].neighbors[0] = pair;
							heapify_subroot(points[from_index].neighbors, points[from_index].n_neighbors, 0);
						}
						omp_unset_lock(&points[from_index].lock);

						// Lock the to point to perform update.
						pair = {to_index, from_index, distance};
						omp_set_lock(&points[to_index].lock);
						if (points[to_index].n_neighbors < 100)
							heap_insert(points[to_index].neighbors, points[to_index].n_neighbors, pair);
						else if (points[to_index].neighbors[0].distance > distance) {
							points[to_index].neighbors[0] = pair;
							heapify_subroot(points[to_index].neighbors, points[to_index].n_neighbors, 0);
						}
						omp_unset_lock(&points[to_index].lock);
					}
				}
			}

			// Write this thread's topP part to topP pairs.
			memcpy(topP_heap, thr_heap, sizeof(Pair) * thr_heap_size);

			//printf("[thread %u]: TD iteration complete. thr_heap_size = %u.\n", thr_id, thr_heap_size);

			++riter;

			#pragma omp barrier
		}

		//printf("[thread %u]: Threads' are joining.\n", thr_id);
		unordered_map<uint32_t, vector<uint32_t>>& curr_BU_level = *riter;

		// The topP pairs have been calculated. Time to populate their from and to candidates.

		for (uint32_t i = 0; i < n_topP; ++i) {
			if (curr_BU_level.count(topP_pairs[i].from_id)) {
				//printf("[thread %u]: Yes! I know about %u. It represents %lu points.\n", thr_id, topP_pairs[i].from_id, curr_BU_level[topP_pairs[i].from_id].size());

				memcpy(&from_cands[i * (partition_size + 1) + 1], curr_BU_level[topP_pairs[i].from_id].data(), sizeof(uint32_t) * curr_BU_level[topP_pairs[i].from_id].size());
				from_cands[i * (partition_size + 1)] = curr_BU_level[topP_pairs[i].from_id].size();
			}
			if (curr_BU_level.count(topP_pairs[i].to_id)) {
				//printf("[thread %u]: Yes! I know about %u. It represents %lu points.\n", thr_id, topP_pairs[i].to_id, curr_BU_level[topP_pairs[i].to_id].size());

				memcpy(&to_cands[i * (partition_size + 1) + 1], curr_BU_level[topP_pairs[i].to_id].data(), sizeof(uint32_t) * curr_BU_level[topP_pairs[i].to_id].size());
				to_cands[i * (partition_size + 1)] = curr_BU_level[topP_pairs[i].to_id].size();
			}
		}

		#pragma omp barrier

		for (uint32_t i = 0; i < P; ++i) {
			// Perform the all pairs cross product on from candidates.
			const uint32_t n_from_cands = from_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1)];
			const uint32_t n_to_cands = to_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1)];
			for (uint32_t i_cand = 0; i_cand < n_from_cands; ++i_cand) {
				for (uint32_t j_cand = i + 1; j_cand < n_from_cands; ++j_cand) {
					uint32_t from_index = from_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + i_cand];
					uint32_t to_index = from_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + j_cand];

					float distance = euclidean_distance_aprox(points[from_index], points[to_index]);

					pair = {from_index, to_index, distance};

					// Lock the from point to perform update.
					omp_set_lock(&points[from_index].lock);
					if (points[from_index].n_neighbors < 100)
						heap_insert(points[from_index].neighbors, points[from_index].n_neighbors, pair);
					else if (points[from_index].neighbors[0].distance > distance) {
						points[from_index].neighbors[0] = pair;
						heapify_subroot(points[from_index].neighbors, points[from_index].n_neighbors, 0);
					}
					omp_unset_lock(&points[from_index].lock);

					// Lock the to point to perform update.
					pair = {to_index, from_index, distance};
					omp_set_lock(&points[to_index].lock);
					if (points[to_index].n_neighbors < 100)
						heap_insert(points[to_index].neighbors, points[to_index].n_neighbors, pair);
					else if (points[to_index].neighbors[0].distance > distance) {
						points[to_index].neighbors[0] = pair;
						heapify_subroot(points[to_index].neighbors, points[to_index].n_neighbors, 0);
					}
					omp_unset_lock(&points[to_index].lock);
				}
			}

			// Perform all pairs cross product on from-to candidates.
			for (uint32_t i_cand = 0; i_cand < n_from_cands; ++i_cand) {
				for (uint32_t j_cand = 0; j_cand < n_to_cands; ++j_cand) {
					uint32_t from_index = from_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + i_cand];
					uint32_t to_index = to_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + j_cand];

					float distance = euclidean_distance_aprox(points[from_index], points[to_index]);

					pair = {from_index, to_index, distance};

					// Lock the from point to perform update.
					omp_set_lock(&points[from_index].lock);
					if (points[from_index].n_neighbors < 100)
						heap_insert(points[from_index].neighbors, points[from_index].n_neighbors, pair);
					else if (points[from_index].neighbors[0].distance > distance) {
						points[from_index].neighbors[0] = pair;
						heapify_subroot(points[from_index].neighbors, points[from_index].n_neighbors, 0);
					}
					omp_unset_lock(&points[from_index].lock);

					// Lock the to point to perform update.
					pair = {to_index, from_index, distance};
					omp_set_lock(&points[to_index].lock);
					if (points[to_index].n_neighbors < 100)
						heap_insert(points[to_index].neighbors, points[to_index].n_neighbors, pair);
					else if (points[to_index].neighbors[0].distance > distance) {
						points[to_index].neighbors[0] = pair;
						heapify_subroot(points[to_index].neighbors, points[to_index].n_neighbors, 0);
					}
					omp_unset_lock(&points[to_index].lock);
				}
			}

			// Perform all pairs cross product on to candidates.
			for (uint32_t i_cand = 0; i_cand < n_to_cands; ++i_cand) {
				for (uint32_t j_cand = i + 1; j_cand < n_to_cands; ++j_cand) {
					uint32_t from_index = to_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + i_cand];
					uint32_t to_index = to_cands[heap_start_index * (partition_size + 1) + i * (partition_size + 1) + 1 + j_cand];

					float distance = euclidean_distance_aprox(points[from_index], points[to_index]);

					pair = {from_index, to_index, distance};

					// Lock the from point to perform update.
					omp_set_lock(&points[from_index].lock);
					if (points[from_index].n_neighbors < 100)
						heap_insert(points[from_index].neighbors, points[from_index].n_neighbors, pair);
					else if (points[from_index].neighbors[0].distance > distance) {
						points[from_index].neighbors[0] = pair;
						heapify_subroot(points[from_index].neighbors, points[from_index].n_neighbors, 0);
					}
					omp_unset_lock(&points[from_index].lock);

					// Lock the to point to perform update.
					pair = {to_index, from_index, distance};
					omp_set_lock(&points[to_index].lock);
					if (points[to_index].n_neighbors < 100)
						heap_insert(points[to_index].neighbors, points[to_index].n_neighbors, pair);
					else if (points[to_index].neighbors[0].distance > distance) {
						points[to_index].neighbors[0] = pair;
						heapify_subroot(points[to_index].neighbors, points[to_index].n_neighbors, 0);
					}
					omp_unset_lock(&points[to_index].lock);
				}
			}
		}
	}

	free(last_BU_level_representatives);
	free(topP_pairs);
	free(from_cands);
	free(to_cands);
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
	uint32_t partition_size = 150;
	// The number of clusters to create per partition.
	uint32_t n_clusters = 4;
	// Number of top object pairs for each TD iteration.
	uint32_t P = 5000;

	printf("Hyperparameters selected:\n");
	printf("\tMinimum partition size = %u\n", partition_size);
	printf("\tClusters to create per partition = %u\n", n_clusters);
	printf("\tTopP pair to check = %u\n", P);

	// Read the dataset from the dummy-data.txt file.
	// The returned memory should be free'd.
	auto [points, n_points] = read_dataset("dummy-data.bin");

	// Perform near neighbor join algorithm.
	printf("===== Near Neighbor Join (start) =====\n");
	near_neighbor_join(points, n_points, partition_size, n_clusters, P);
	printf("===== Near Neighbor Join (exits) =====\n");

	//for (uint32_t i = 0; i < n_points; ++i)
		//printf("%u-th point has %u neighbors.\n", i, points[i].n_neighbors);

	// Write the knng to disk.
	write_knng(points, n_points, "output.bin");

	free(points);

	return EXIT_SUCCESS;
}
