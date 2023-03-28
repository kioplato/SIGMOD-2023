#pragma once

#include <vector>
#include <cstdint>

using namespace std;

// Shorter point related types.
class point_t;

typedef vector<point_t> points_t;
typedef vector<point_t*> point_ptrs_t;
typedef vector<const point_t*> point_cptrs_t;

// Shorter cluster related types.
class cluster_t;

typedef vector<cluster_t> clusters_t;
typedef vector<cluster_t*> cluster_ptrs_t;
typedef vector<const cluster_t*> cluster_cptrs_t;

// Shorter knng type.
typedef vector<vector<uint32_t>> knng_t;
