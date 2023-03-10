#include <algorithm>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "assert.h"
#include "io.h"

using namespace std;

#define _INT_MAX 2147483640

vector<uint32_t> CalculateOneKnn(const vector<vector<float>> &data,
		const vector<uint32_t> &sample_indexes, const uint32_t id)
{
	// Max heap. This way we know which is the furthest point from @id.
	priority_queue<pair<float, uint32_t>> top_candidates;

	double lower_bound = _INT_MAX;

	// Iterate over samples.
	for (unsigned i = 0; i < sample_indexes.size(); i++) {
		// Get the current sample index in @data.
		uint32_t sample_id = sample_indexes[i];

		// Skip itself. A point can't have itself as a neighbor.
		if (id == sample_id) continue;

		// Calculate the distance between @id and @sample_id.
		double dist = EuclideanDistance(data[id], data[sample_id]);

		// Only keep the top 100 nearest neighbors. k=100.
		if (top_candidates.size() < 100 || dist < lower_bound) {
			top_candidates.push(std::make_pair(dist, sample_id));
			if (top_candidates.size() > 100)
				top_candidates.pop();

			lower_bound = top_candidates.top().first;
		}
	}

	// Write the nns of the current point @id.
	vector<uint32_t> knn; knn.reserve(100);
	while (!top_candidates.empty()) {
		knn.emplace_back(top_candidates.top().second);
		top_candidates.pop();
	}
	reverse(knn.begin(), knn.end());

	return knn;
}

void ConstructKnng(const vector<vector<float>> &data,
		const vector<uint32_t> &sample_indexes,
		vector<vector<uint32_t>> &knng)
{
	knng.resize(data.size());
#pragma omp parallel for
	for (uint32_t n = 0; n < knng.size(); ++n)
		knng[n] = CalculateOneKnn(data, sample_indexes, n);
}

int main(int argc, char **argv)
{
	string source_path = "dummy-data.bin";

	// Also accept other path for source data.
	if (argc > 1)
		source_path = string(argv[1]);

	// Read data points.
	vector<vector<float>> nodes;
	ReadBin(source_path, nodes);

	// Sample points for greedy search.
	std::default_random_engine rd;
	std::mt19937 gen(rd());  // Mersenne twister MT19937.
	vector<uint32_t> sample_indexes(nodes.size());
	iota(sample_indexes.begin(), sample_indexes.end(), 0);
	shuffle(sample_indexes.begin(), sample_indexes.end(), gen);
	// For evaluation dataset, keep more points.
	if (sample_indexes.size() > 100000)
		sample_indexes.resize(100000);

	// Knng constuction.
	vector<vector<uint32_t>> knng;
	ConstructKnng(nodes, sample_indexes, knng);

	// Save to ouput.bin file.
	SaveKNNG(knng);

	return 0;
}
