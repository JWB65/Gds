#include "Transform.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>


static gds_pair
AddPairs(gds_pair one, gds_pair two)
{
	return {one.x + two.x, one.y + two.y};
}

static gds_pair
SubtractPairs(gds_pair one, gds_pair two)
{
	return {one.x - two.x, one.y - two.y};
}

static gds_pair
RotatePair(const gds_pair in, double angle, double mag, bool Rx)
{
	double sign = Rx ? -1. : 1.;

	double s = sin(angle);
	double c = cos(angle);

	int64_t x = (int64_t)(mag * (in.x * c - sign * in.y * s));
	int64_t y = (int64_t)(mag * (in.x * s + sign * in.y * c));

	//assert(std::in_range<int32_t>(x) && std::in_range<int32_t>(y)); // They need to be in the GDSII database range of 4 byte signed integers

	return { x, y };
}

gds_pair
transform_pair(const gds_pair in, const gds_transform* transform, bool inv)
{
	gds_pair p;

	if (inv)
	{
		p = RotatePair(SubtractPairs(in, transform->translation), -transform->angle, 1. / transform->magnification, transform->mirror != 0x0000);
	} else
	{
		p = AddPairs(transform->translation, RotatePair(in, transform->angle, transform->magnification, transform->mirror != 0x0000));
	}

	return p;
}

void
transform_pairs(gds_pair* out, const gds_pair* in, int npairs, const gds_transform* tra, bool inv)
{
	// Transforms an array of pairs (@out needs to be allocated by caller)

	assert(out && in);

	for (int i = 0; i < npairs; ++i)
	{
		out[i] = transform_pair(in[i], tra, inv);
	}
}
