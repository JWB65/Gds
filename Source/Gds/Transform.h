#pragma once

#include "Pair.h"

#include <stdbool.h>

typedef struct Transform
{
	short mirror;
	gds_pair translation;
	double magnification, angle;
} gds_transform;

gds_pair transform_pair(const gds_pair in, const gds_transform* transform, bool inv);

void transform_pairs(gds_pair* out, const gds_pair* in, int npairs, const gds_transform* transform, bool inv);
