#include <iostream>
#include <cstdint>
#include <vector>
#include "point.hpp"

using namespace std;

point_t::point_t(size_t n_dims) : _coordinates(n_dims, 0.0)
{
	/* Empty. */
}

point_t::point_t(uint32_t id, const vector<float> coordinates)
: _id(id), _coordinates(coordinates)
{
	/* Empty. */
}

size_t point_t::n_dims() const
{
	return _coordinates.size();
}

uint32_t point_t::id() const
{
	return _id;
}

uint32_t point_t::cluster_id() const
{
	return _cluster_id;
}

float point_t::operator[] (size_t index) const
{
	return _coordinates[index];
}

float& point_t::operator[] (size_t index)
{
	return _coordinates[index];
}

void point_t::cluster_id(uint32_t id)
{
	_cluster_id = id;
}

const cluster_t* point_t::cluster() const
{
	return _cluster;
}

void point_t::cluster(const cluster_t* cluster)
{
	_cluster = cluster;
}

void point_t::print(ofstream ofs) const
{
	uint32_t n_dims = _coordinates.size();

	for (uint32_t c_dim = 0; c_dim < n_dims; ++c_dim)
		ofs << _coordinates.at(c_dim) << ' ';

	ofs << endl;
}
