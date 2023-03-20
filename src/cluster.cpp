#include "cluster.hpp"
#include "helpers.hpp"

cluster_t::cluster_t(uint32_t cluster_id, const vector<float>& coords)
: _cluster_id(cluster_id), _centroid(0, coords)
{
	/* Empty. */
}

uint32_t cluster_t::id() const
{
	return _cluster_id;
}

const point_t& cluster_t::centroid() const
{
	return _centroid;
}

void cluster_t::centroid(const point_t& centroid)
{
	_centroid = centroid;
}

void cluster_t::recenter()
{
	// We don't have any points to use for the computation of centroid.
	if (_points.empty()) return;

	// The dimensions of each point.
	size_t n_dims = _points.front()->coords().size();

	// The coordinates of the new centroid.
	vector<float> centroid(n_dims);

	for (size_t c_dim = 0; c_dim < n_dims; ++c_dim) {
		double dim_value = 0.0;

		#pragma omp parallel for reduction(+: dim_value)
		for (const auto& point : _points)
			dim_value += point->coords().at(c_dim);

		centroid[c_dim] = dim_value / _points.size();
	}

	_centroid = point_t(0, centroid);
}

void cluster_t::add_point(const point_t* point)
{
	#pragma omp critical
	_points.push_back(point);
}

bool cluster_t::remove_point(uint32_t point_id)
{
	for (auto iter = _points.begin(); iter != _points.end(); ++iter) {
		if ((*iter)->id() != point_id)
			continue;

		_points.erase(iter);
		return true;
	}

	return false;
}

const point_cptrs_t& cluster_t::points() const
{
	return _points;
}

void cluster_t::clear()
{
	_points.clear();
}

void cluster_t::print(ostream& outstream, string indent, bool print_points,
		bool print_coords) const
{
	outstream << indent << "Cluster:" << endl;
	outstream << indent << "\tID = " << _cluster_id << endl;
	outstream << indent << "\tAddress = " << this << endl;
	outstream << indent << "\t# points = " << _points.size() << endl;
	outstream << indent << "\tCentroid:" << endl;
	_centroid.print(outstream, indent + "\t\t", print_coords);
	if (print_points) {
		outstream << indent << "\tPoints:" << endl;
		for (const point_t* point : _points)
			point->print(outstream, indent + "\t\t", print_coords);
	}
}
