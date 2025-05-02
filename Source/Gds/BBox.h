#pragma once

#include "Pair.h"
#include "Transform.h"

#include <stdint.h>

struct gds_bbox
{
	int64_t xmin, ymin, xmax, ymax;
};

void bbox_init(gds_bbox* self);

uint64_t bbox_size(const gds_bbox* target);

void bbox_fit_point(gds_bbox* self, gds_pair p);

void bbox_fit_points(gds_bbox* self, const gds_pair* pairs, int npairs);

void bbox_fit_bbox(gds_bbox* self, const gds_bbox* other);

bool bbox_check_overlap(const gds_bbox* b, const gds_bbox* a);

gds_bbox bbox_transform(const gds_bbox* in, const gds_transform* transform, bool inv);
