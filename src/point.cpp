#include <iostream>
#include <cstdint>
#include <vector>
#include "point.hpp"
#include "cluster.hpp"

using namespace std;

point_t::point_t(const uint32_t& id, const vector<float>& coordinates)
: _id(id), _coordinates(coordinates)
{
	/* Empty. */
}

uint32_t point_t::id() const
{
	return _id;
}

const cluster_ptrs_t& point_t::clusters() const
{
	return _clusters;
}

void point_t::clusters(const cluster_ptrs_t& clusters)
{
	_clusters = clusters;
}

const vector<float>& point_t::coords() const
{
	return _coordinates;
}

void point_t::print(ostream& outstream, string indent, bool print_coords) const
{
	uint32_t n_dims = _coordinates.size();

	outstream << indent << "Point:" << endl;
	outstream << indent << "\tID = " << _id << endl;
	outstream << indent << "\tAddress = " << this << endl;

	outstream << indent << "\tNearest Clusters addresses:" << endl;
	for (const cluster_t* cluster : _clusters)
		outstream << indent << "\t\tAddress=" << cluster << ",ID=" << cluster->id() << endl;
	if (_clusters.empty())
		outstream << indent << "\t\tNo nearest clusters set." << endl;;

	if (print_coords) {
		outstream << indent << "\tCoordinates:" << endl;
		for (uint32_t c_dim = 0; c_dim < n_dims; ++c_dim) {
			if (c_dim % 10 == 0)
				outstream << indent + "\t\t";

			outstream << _coordinates.at(c_dim) << ' ';

			if (c_dim % 10 == 9)
				outstream << endl;
		}
	}

	outstream << endl;
}
