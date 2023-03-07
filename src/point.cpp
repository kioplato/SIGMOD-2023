#include <iostream>
#include <cstdint>
#include <vector>
#include "point.hpp"
#include "cluster.hpp"

using namespace std;

point_t::point_t(const uint32_t& id, const vector<float>& coordinates)
: _id(id), _cluster(NULL), _coordinates(coordinates)
{
	/* Empty. */
}

uint32_t point_t::id() const
{
	return _id;
}

const cluster_t* point_t::cluster() const
{
	return _cluster;
}

void point_t::cluster(const cluster_t* cluster)
{
	_cluster = cluster;
}

const vector<float>& point_t::coords() const
{
	return _coordinates;
}

void point_t::print(ostream& outstream, string indent) const
{
	uint32_t n_dims = _coordinates.size();

	outstream << indent << "Point:" << endl;
	outstream << indent << "\tID = " << _id << endl;
	outstream << indent << "\tCluster addr = " << _cluster << endl;
	if (_cluster)
		outstream << indent << "\tCluster ID through ptr = " << _cluster->id() << endl;
	outstream << indent << "\tCoordinates:" << endl;
	for (uint32_t c_dim = 0; c_dim < n_dims; ++c_dim) {
		if (c_dim % 10 == 0)
			outstream << indent + "\t\t";

		outstream << _coordinates.at(c_dim) << ' ';

		if (c_dim % 10 == 9)
			outstream << endl;
	}

	outstream << endl;
}
