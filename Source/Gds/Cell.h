#pragma once

#include "BBox.h"
#include "Pair.h"

#include "stdint.h"

#include <vector>

#define GDS_MAX_CELL_NAME 32

struct gds_boundary
{
	uint16_t layer;
	gds_pair* pairs;
	int npairs;
	gds_bbox bbox;
};

struct gds_path
{
	uint16_t layer, pathtype;
	uint32_t width;

	gds_pair* pairs;
	int npairs;

	// When a gds file is read, path elements will be expanded into a normal boundary element
	gds_pair* epairs;
	int nepairs;

	gds_bbox bbox;
};

// Forward declaration type gds_cell because of circular dependency
struct gds_cell;

struct gds_sref
{
	uint16_t strans;
	double angle, mag;
	gds_pair origin;
	char sname[GDS_MAX_CELL_NAME + 1];
	gds_cell* cell;
};

struct gds_aref
{
	double angle, mag;
	uint16_t strans;
	int ncols, nrows;
	gds_pair vectors[3];
	char sname[GDS_MAX_CELL_NAME + 1];
	gds_cell* cell;
};

struct gds_cell
{
public:
	gds_cell();
	~gds_cell();

	char name[GDS_MAX_CELL_NAME + 1];

	std::vector<gds_boundary*> *boundaries;
	std::vector<gds_path*> *paths;
	std::vector<gds_sref*> *srefs;
	std::vector<gds_aref*> *arefs;

	gds_bbox bbox; // Is recursively calculated after loading the database
	bool initialized; // Is set true when member @bbox is initialized
};