#pragma once

#include "BBox.h"
#include "Pair.h"

#include <stdint.h>

#include <vector>

class gds_polygon {
public:

	// The constructor requires an already memory allocated array of type gds_pair
	gds_polygon(gds_pair* pairs, int npairs, gds_bbox box, uint16_t layer);

	// The destructor takes care of deleting the allocated memory in member pairs
	~gds_polygon();

	gds_pair* pairs; // Coordinates of vertices in database units
	int npairs; // Number of coordinates (note GDSII polygons are closed polygons)

	uint16_t layer; // Layer in the GDSII database to which the polygon belongs

	gds_bbox bbox; // Bounding box of the polygon
};

typedef std::vector<gds_polygon*> gds_polyset;


void gds_polyset_clear(gds_polyset* pset);

void gds_print_polyset(gds_polyset* pset);

/*
	Print to stdout all polygons elements of a polygon set
 */
void gds_print_polyset(gds_polyset* pset);
