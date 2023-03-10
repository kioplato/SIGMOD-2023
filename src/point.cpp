#include <cstdint>
#include <vector>
#include "point.hpp"

using namespace std;

point_t::point_t(size_t n_dims) : _coordinates(n_dims, 0.0)
{
	/* Empty. */
}

point_t::point_t(uint32_t id, const vector<double>& coordinates)
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

double point_t::operator[] (size_t index) const
{
	return _coordinates[index];
}

double& point_t::operator[] (size_t index)
{
	return _coordinates[index];
}

void point_t::cluster_id(uint32_t id)
{
	_cluster_id = id;
}
