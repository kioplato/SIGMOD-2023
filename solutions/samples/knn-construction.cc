/**
 *  Naive baseline for construction a KNN Graph.
 *  Randomly select 100 neighbors from a 10k subset.
 */

#include <sys/time.h>

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "assert.h"
#include "io.h"

using namespace std;

#define _INT_MAX 2147483640

float EuclideanDistance(const vector<float> &lhs, const vector<float> &rhs)
{
	float ans = 0.0;
	unsigned lensDim = 100;

	for (unsigned i = 0; i < lensDim; ++i)
		ans += (lhs[i] - rhs[i]) * (lhs[i] - rhs[i]);

	return ans;
}

vector<uint32_t> CalculateOneKnn(const vector<vector<float>> &data,
		const vector<uint32_t> &sample_indexes,
		const uint32_t id)
{
	std::priority_queue<std::pair<float, uint32_t>> top_candidates;
	float lower_bound = _INT_MAX;
	for (unsigned i = 0; i < sample_indexes.size(); i++) {
		uint32_t sample_id = sample_indexes[i];
		if (id == sample_id) continue;  // skip itself.
		float dist = EuclideanDistance(data[id], data[sample_id]);

		// only keep the top 100
		if (top_candidates.size() < 100 || dist < lower_bound) {
			top_candidates.push(std::make_pair(dist, sample_id));
			if (top_candidates.size() > 100) {
				top_candidates.pop();
			}

			lower_bound = top_candidates.top().first;
		}
	}

	vector<uint32_t> knn;
	while (!top_candidates.empty()) {
		knn.emplace_back(top_candidates.top().second);
		top_candidates.pop();
	}

	return knn;
}

void ConstructKnng(const vector<vector<float>> &data,
		const vector<uint32_t> &sample_indexes,
		vector<vector<uint32_t>> &knng)
{
	knng.resize(data.size());

#pragma omp parallel for schedule(guided, 1)
	for (uint32_t n = 0; n < knng.size(); ++n)
		knng[n] = CalculateOneKnn(data, sample_indexes, n);
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		cerr << "error: must provide two arguments: dataset and sample size" << endl;
		exit(1);
	}

	string source_path(argv[1]);
	size_t sample_size(atoll(argv[2]));

	// Read data points
	vector<vector<float>> nodes;
	ReadBin(source_path, nodes);

	// Sample points for greedy search
	vector<uint32_t> sample_indexes(nodes.size());
	iota(sample_indexes.begin(), sample_indexes.end(), 0);

	default_random_engine rd;
	mt19937 gen(rd());
	shuffle(sample_indexes.begin(), sample_indexes.end(), gen);

	/*
	 * If the dataset's cardinality is smaller than the sample size, we
	 * limit the sample's size to the dataset's cardinality.
	 */
	sample_size = min(nodes.size(), sample_size);
	sample_indexes.resize(sample_size);

	// Knng constuction
	vector<vector<uint32_t>> knng;
	ConstructKnng(nodes, sample_indexes, knng);

	// Save to ouput.bin
	SaveKNNG(knng);

	return EXIT_SUCCESS;
}
