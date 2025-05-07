#include "BBox.h"

inline static
int64_t min(int64_t const x, int64_t const y)
{
	return y < x ? y : x;
}

gds_bbox::gds_bbox()
	: xmin(INT64_MAX), ymin(INT64_MAX), xmax(INT64_MIN), ymax(INT64_MIN)
{
}

gds_bbox::gds_bbox(uint64_t xmin, uint64_t ymin, uint64_t xmax, uint64_t ymax)
	: xmin(xmin), ymin(ymin), xmax(xmax), ymax(ymax)
{
}

uint64_t gds_bbox::size()
{
	return min(xmax - xmin, ymax - ymin);
}

void gds_bbox::fit_point(gds_pair p)
{
	if (p.x < xmin)
		xmin = p.x;

	if (p.x > xmax)
		xmax = p.x;

	if (p.y < ymin)
		ymin = p.y;

	if (p.y > ymax)
		ymax = p.y;
}

void gds_bbox::fit_points(const gds_pair* pairs, int npairs)
{
	for (int i = 0; i < npairs; ++i){
		fit_point(pairs[i]);
	}
}

void gds_bbox::fit_bbox(const gds_bbox& other)
{
	fit_point({other.xmin, other.ymin});
	fit_point({other.xmin, other.ymax});
	fit_point({other.xmax, other.ymax});
	fit_point({other.xmax, other.ymin});
}

bool gds_bbox::check_overlap(const gds_bbox& b) const
{
	bool overlap = (xmin < b.xmax) && (xmax > b.xmin) && (ymax > b.ymin) && (ymin < b.ymax);

	return overlap;

}

gds_bbox gds_bbox::transform(const gds_transform& transform, bool inv) const
{
	// Return a transformed bounding box

	gds_pair pairs[4];
	gds_pair tpairs[4];

	pairs[0] = {xmin, ymin};
	pairs[1] = {xmin, ymax};
	pairs[2] = {xmax, ymax};
	pairs[3] = {xmax, ymin};

	transform_pairs(tpairs, pairs, 4, &transform, inv);

	gds_bbox out;

	out.fit_points(tpairs, 4);

	return out;
}
