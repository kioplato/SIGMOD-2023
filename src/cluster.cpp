#include "cluster.hpp"

cluster_t::cluster_t(uint32_t cluster_id, const point_t& centroid)
: _cluster_id(cluster_id), _centroid(centroid)
{
	/* Empty. */
}

uint32_t cluster_t::id() const
{
	return _cluster_id;
}

void cluster_t::add_member(const point_t& point)
{
	_points.push_back(point);
	_points.back().cluster_id(_cluster_id);
}

bool cluster_t::remove_member(uint32_t point_id)
{
	for (auto iter = _points.begin(); iter != _points.end(); ++iter) {
		if (iter->id() != point_id) continue;

		_points.erase(iter);
		return true;
	}

	return false;
}

size_t cluster_t::size() const
{
	return _points.size();
}

const point_t& cluster_t::operator[] (size_t index) const
{
	return _points[index];
}

const point_t& cluster_t::centroid() const
{
	return _centroid;
}

void cluster_t::reset_members()
{
	_points.clear();
}

void cluster_t::centroid(const point_t& centroid)
{
	_centroid = centroid;
}

const vector<point_t>& cluster_t::members() const
{
	return _points;
}

void cluster_t::recenter()
{
	if (_points.empty()) return;

	size_t n_dims = _points.front.size();

	point_t new_centroid;

	for (size_t c_dim = 0; c_dim < n_dims; ++c_dim) {
		double dim_value = 0.0;

#pragma omp parallel for reduction(+: dim_value)
		for (const auto& point : _points)
			dim_value += point[c_dim];

		new_centroid[c_dim] = dim_value / _points.size();
	}

	_centroid = new_centroid;
}
