#pragma once

#include "BBox.h"
#include "Pair.h"

#include <stdint.h>

#include <vector>

class gds_polygon{
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

class gds_polyset{
public:
	gds_polyset();

	~gds_polyset();

	void print();

	/*
	Write all polygon elements to a GDS file

	All polygons are saved as boundary elements in top cell "TOP"

	@dbunit_size_uu: database size in user units
	@dbunit_size_in_m: database size in meter
	*/
	int write(const wchar_t* dest, double dbunit_size_uu, double dbunit_size_in_m);

	void add(gds_polygon* p);

	size_t size();

	void clear();

	std::vector<gds_polygon*>* get() { return set; }

private:
	std::vector<gds_polygon*>* set;
};
