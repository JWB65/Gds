#include "BBox.h"

inline static
int64_t min(int64_t const x, int64_t const y)
{
	return y < x ? y : x;
}

void bbox_init(gds_bbox* self)
{
	*self = {INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN};
}

uint64_t bbox_size(const gds_bbox* target)
{
	return min(target->xmax - target->xmin, target->ymax - target->ymin);
}

void bbox_fit_point(gds_bbox* self, gds_pair p)
{
	if (p.x < self->xmin)
		self->xmin = p.x;

	if (p.x > self->xmax)
		self->xmax = p.x;

	if (p.y < self->ymin)
		self->ymin = p.y;

	if (p.y > self->ymax)
		self->ymax = p.y;
}

void bbox_fit_points(gds_bbox* self, const gds_pair* pairs, int npairs)
{
	// Adjust the size of a bounding box to fit a array of pairs

	for (int i = 0; i < npairs; ++i) {
		bbox_fit_point(self, pairs[i]);
	}
}

void bbox_fit_bbox(gds_bbox* self, const gds_bbox* other)
{
	bbox_fit_point(self, {other->xmin, other->ymin});
	bbox_fit_point(self, {other->xmin, other->ymax});
	bbox_fit_point(self, {other->xmax, other->ymax});
	bbox_fit_point(self, {other->xmax, other->ymin});
}

bool bbox_check_overlap(const gds_bbox* b, const gds_bbox* a)
{
	// Check if there's *possible* overlap between two bounding boxes

	bool overlap = (a->xmin < b->xmax) && (a->xmax > b->xmin) && (a->ymax > b->ymin) && (a->ymin <
		b->ymax);

	return overlap;
}

gds_bbox bbox_transform(const gds_bbox* in, const gds_transform* transform, bool inv)
{
	// Return a transformed bounding box

	gds_pair pairs[4];
	gds_pair tpairs[4];

	pairs[0] = {in->xmin, in->ymin};
	pairs[1] = {in->xmin, in->ymax};
	pairs[2] = {in->xmax, in->ymax};
	pairs[3] = {in->xmax, in->ymin};

	transform_pairs(tpairs, pairs, 4, transform, inv);

	gds_bbox out;
	bbox_init(&out);

	bbox_fit_points(&out, tpairs, 4);

	return out;
}
