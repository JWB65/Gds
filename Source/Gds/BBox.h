#pragma once

#include "Pair.h"
#include "Transform.h"

#include <stdint.h>

class gds_bbox{
public:
	gds_bbox();

	gds_bbox(uint64_t xmin, uint64_t ymin, uint64_t xmax, uint64_t ymax);

	uint64_t size();

	void fit_point(gds_pair p);

	void fit_points(const gds_pair* pairs, int npairs);

	void fit_bbox(const gds_bbox& other);

	bool check_overlap(const gds_bbox& other) const;

	gds_bbox transform(const gds_transform& transform, bool inv) const;

	int64_t xmin, ymin, xmax, ymax;
};
