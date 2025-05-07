#include "gds.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

// Line pair standard form ax + by = c
typedef struct {
	double a, b, c;
} Line;

static
bool test_lines_parallel(Line line1, Line line2)
{
	return ((line1.a * line2.b - line2.a * line1.b) == 0.);
}

static
gds_pair intersect_lines(Line line1, Line line2)
{
	// Line-Line intersection (homogeneous coordinates):
	// (B1C2 - B2C1, A2C1 - A1C2, A1B2 - A2B1)

	// Third homogeneous coordinate of intersection
	double w = line1.a * line2.b - line2.a * line1.b;

	assert(w != 0.); // Lines do not intersect in a finite point

	// Intersection
	double x = (line1.b * line2.c - line2.b * line1.c) / w;
	double y = (line2.a * line1.c - line1.a * line2.c) / w;

	return {(int)x, (int)y};
}

static
gds_pair project(gds_pair p, Line line)
{
	// normal to Ax + By + C = 0 through (x1, y1)
	// A'x + B'y + C' = 0 with
	// A' = B  B' = -A  C' = Ay1 - Bx1

	Line normal = {line.b, -line.a, line.a * p.y - line.b * p.x};

	// Intersect the normal through (pair.x, pair.y) with the line
	return intersect_lines(line, normal);
}

static
gds_pair extend(gds_pair tail, gds_pair head, double length)
{
	// Extends a vector given by @tail and @head pair the tail direction by@length

	double segx, segy, norm;
	gds_pair out = {tail.x, tail.y};

	segx = (double)(tail.x - head.x);
	segy = (double)(tail.y - head.y);
	norm = sqrt(segx * segx + segy * segy);

	if (norm != 0.0) {
		out.x = tail.x + (int)((length / norm) * segx);
		out.y = tail.y + (int)((length / norm) * segy);
	}

	return out;
}

int gds_expand_path(gds_pair* out, const gds_pair* in, int npairs_in, uint32_t width, uint16_t pathtype)
{
	// Expands a GDS path and outputs as array of pairs. @out needs to be allocated with 2 * @ninpairs + 1 pairs by caller.

	// Path type 1:

	/*
		  O             O


		  C             C

		  C             C
	*/

	if (npairs_in < 2)
		return EXIT_FAILURE;

	// The half width to add each side
	double hwidth = width / 2.0;

	Line* mlines = (Line*)malloc((npairs_in - 1) * sizeof(Line));
	Line* plines = (Line*)malloc((npairs_in - 1) * sizeof(Line));

	for (int i = 0; i < npairs_in - 1; i++) {
		double a, b, c, c_trans;

		// Line through (x1, y1) & (x2, y2)
		//    Ax + By + C = 0
		// with
		//    A = y2 - y1
		//    B = -(x2 - x1)
		//    C = -By1 - Ax1
		a = (double)(in[i + 1].y - in[i].y);
		b = -(double)(in[i + 1].x - in[i].x);

		// Return failure if the two ends of the segment coincide
		if (a == 0. && b == 0.) {
			free(plines);
			free(mlines);
			return EXIT_FAILURE;
		}

		c = -b * in[i].y - a * in[i].x;

		// Line parallel to Ax + By + C = 0 at distance d: Ax + By + (C +/- d * sqrt(A^2 + B^2))
		c_trans = hwidth * sqrt(a * a + b * b);

		plines[i].a = mlines[i].a = a;
		plines[i].b = mlines[i].b = b;

		// Coefficient pair standard form of parallel segments
		plines[i].c = c + c_trans;
		mlines[i].c = c - c_trans;
	}

	// Head points

	gds_pair end_point;
	if (pathtype == 2) {
		if (in[0].x != in[1].x || in[0].x != in[1].y)
			end_point = extend(in[0], in[1], hwidth);
		else {
			// The two coordinates are the same: can not extend
			free(plines);
			free(mlines);
			return EXIT_FAILURE;
		}
	} else {
		end_point = in[0];
	}
	out[0] = project(end_point, plines[0]);
	out[2 * npairs_in - 1] = project(end_point, mlines[0]);
	out[2 * npairs_in] = out[0];

	// Middle points

	for (int i = 1; i < npairs_in - 1; ++i) {
		bool result = !test_lines_parallel(plines[i - 1], plines[i]) &&
			!test_lines_parallel(mlines[i - 1], mlines[i]);

		if (result) {
			out[i] = intersect_lines(plines[i - 1], plines[i]);
			out[2 * npairs_in - 1 - i] = intersect_lines(mlines[i - 1], mlines[i]);
		} else {
			free(plines);
			free(mlines);
			return EXIT_FAILURE;
		}
	}

	// Tail points

	if (pathtype == 2) {
		if (in[npairs_in - 1].x != in[npairs_in - 2].x || in[npairs_in - 1].x != in[npairs_in - 2].y)
			end_point = extend(in[npairs_in - 1], in[npairs_in - 2], hwidth);
		else {
			// The two coordinates are the same: can not extend
			free(plines);
			free(mlines);
			return EXIT_FAILURE;
		}
	} else {
		end_point = in[npairs_in - 1];
	}
	out[npairs_in - 1] = project(end_point, plines[npairs_in - 2]);
	out[npairs_in] = project(end_point, mlines[npairs_in - 2]);

	free(plines);
	free(mlines);

	return EXIT_SUCCESS;
}
