#include <iostream>
#include <cstdint>
#include <omp.h>

#include "input-output.hpp"

struct Pair {
	uint32_t from_id;
	uint32_t to_id;
	float distance;
};

struct Point {
	// Identifies the point uniquely.
	// Range: [0, 4,294,967,295].
	uint32_t id;

	// The BU level of the point.
	// Range: [0, 65,535].
	uint16_t level;

	// How many objects assigned to this point.
	// Range: [0, 65,535].
	uint16_t weight;

	// Cluster center that represents this point.
	// Range: [0, 4,294,967,295].
	uint32_t represent_id;

	// Flexible array idiom.1000000/25
	// Allocate 100 * sizeof(float) more bytes for coordinates.
	// Allocate 100 * sizeof(Pair) more bytes for nearest neighbors.
};

pair<Point*, uint32_t> read_dataset(const string& path)
{
	// Open the dataset from the disk.
	ifstream ifs(path, ios::binary);
	if (!ifs.is_open())
		die("failed to open dataset at ", path);

	// Read number of points in the dataset.
	uint32_t dataset_cardinality;
	ifs.read((char*)&dataset_cardinality, sizeof(uint32_t));

	// Reserve memory for the points.
	// In the paper a point is the same as SajObj in Table II.
	size_t point_bytes = sizeof(Point) + 100 * sizeof(float) + 100 * sizeof(Pair);
	Point *points = (Point*)malloc(point_bytes * dataset_cardinality);

	// The id of the next point we are going to read.
	uint32_t next_point_id = 0;

	// Read one by one the points of the dataset.
	while (ifs.read(((char*)(points + next_point_id)) + sizeof(Point), 100 * sizeof(float)))
		points[next_point_id].id = next_point_id++;

	ifs.close();

	return {points, dataset_cardinality};
}

void bottom_up(Point *points, uint32_t n_points, uint32_t k,
		uint32_t n_parts, uint32_t n_clusters)
{
	// Where the current BU level's points start and end.
	uint32_t index_start = 0;
	uint32_t index_end = n_points - 1;

	// The BU level's remaining points. First BU level is @n_points.
	uint32_t BU_size = n_points;

	bool done = false;

	#pragma omp parallel
	while (!done)
	{
		// The number of total threads.
		const uint32_t n_thr = omp_get_num_threads();
		// The ID of this thread.
		const uint32_t thr_id = omp_get_thread_num();
		// The thread's partition size for this BU level.
		const uint32_t thr_chunk_size = BU_size/n_thr + ((BU_size % n_thr) > thr_id);

		// The thread's start and end index in @points.
		uint32_t thr_index_start = thr_id * thr_chunk_size;
		uint32_t thr_index_end = thr_id * thr_chunk_size + thr_chunk_size - 1;

		// The size of each partition of the thread's chunk.
		uint32_t part_size = thr_chunk_size/n_parts;
		// The first @remainder partitions receive one more item.
		uint32_t remainder = thr_chunk_size % n_parts;

		// Where the next partition will start.
		uint32_t part_start = thr_index_start;

		// Store the indices of the centroids of the thread.
		uint32_t centroids[n_parts * n_clusters];

		// Iterate over the thread's partitions.
		for (uint32_t c_part = 0; c_part < n_parts; ++c_part) {
			// This partition's size.
			uint32_t part_size = part_size + (c_part < remainder);

			// Compute the distance between all pairs of the partition.
			// At the same time compute the initial nearest neighbors.
			float distances[part_size][part_size];
			compute_distances(distances, points, part_start, part_size);

			// Perform kmeans clustering with one iteration.
			// At the same time each point gets assigned a representative.
			kmeans(points, part_start, part_size, centroids, c_part, n_clusters, 1);
		}

		// Wait for everyone to stop reading their points.
		#pragma omp barrier

		// Write the centroids at the end of the buffer @points.
		// This messes up the last points and that's why we barrier'ed.
		memmove_centroids(points, centroids, n_parts * n_clusters);

		// Update bookkeeping variables.
		#pragma omp single
		{
			// Next BU level has different size then the current.
			BU_size = (BU_size * n_clusters) / (n_thr * n_parts);
			BU_size = BU_size / (n_thr * n_parts);
			BU_size = n_thr * n_parts * n_clusters;

			// We want each BU level to have the same partition size.
			n_parts = 

			// Next BU level starts at a new index.
			// The end index remains the same.
			index_start = n_points - BU_size;

			// We are done when @threshold is satisfied.
			done = threshold > BU_size;
		}
	}
}

int main(void)
{
	// Don't use dynamic number of threads according to system load.
	omp_set_dynamic(0);
	// Don't use nested parallelism.
	omp_set_nested(0);
	// The number of threads to create will match to the number of cores.
	omp_set_num_threads(omp_get_num_procs());

	printf("Near Neighbor Join.\n");

	/* The hyperparameters of the problem (Table I). */
	// The nearest neighbors to compute.
	uint32_t k = 100;
	// The number of partitions per thread.
	uint32_t n_parts = 500;
	// The number of clusters to create per partition.
	uint32_t n_clusters = 10;
	// Number of top object pairs for each TD iteration.
	uint32_t P = 5;

	printf("Hyperparameters selected:\n");
	printf("\tNearest neighbors = %u\n", k);
	printf("\tPartitions to create per thread = %u\n", n_parts);
	printf("\tClusters to create per partition = %u\n", n_clusters);
	printf("\tTopP pair to check = %u\n", P);

	// Read the dataset from the dummy-data.txt file.
	// The returned memory should be free'd.
	auto [points, n_points] = read_dataset("dummy-data.bin");

	// Perform the bottom-up phase.
	bottom_up(points, n_points, k, n_parts, n_clusters);

	return EXIT_SUCCESS;
}
