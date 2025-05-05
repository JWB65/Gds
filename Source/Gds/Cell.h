#pragma once

#include "BBox.h"
#include "Pair.h"

#include "stdint.h"

#include <memory>
#include <optional>
#include <vector>

#define GDS_MAX_CELL_NAME 32

struct gds_boundary
{
	uint16_t layer;
	std::vector<gds_pair> pairs;
	gds_bbox bbox;
};

struct gds_path
{
	uint16_t layer, pathtype;
	uint32_t width;

	std::vector<gds_pair> pairs;

	// Path element expanded in a boundary element
	std::vector<gds_pair> epairs;

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

	// Pointer to the cell it's referring to (will be assigned after the db is loaded)
	gds_cell* cell;
};

struct gds_aref
{
	double angle, mag;
	uint16_t strans;
	int ncols, nrows;
	gds_pair vectors[3];
	char sname[GDS_MAX_CELL_NAME + 1];

	// Pointer to the cell it's referring to (will be assigned after the db is loaded)
	gds_cell* cell;
};

struct gds_cell
{
public:
	gds_cell();
	~gds_cell();

	char name[GDS_MAX_CELL_NAME + 1];

	std::vector<std::unique_ptr<gds_boundary>> boundaries;
	std::vector<std::unique_ptr<gds_path>> paths;
	std::vector<std::unique_ptr<gds_sref>> srefs;
	std::vector<std::unique_ptr<gds_aref>> arefs;

	std::optional<gds_bbox> bbox; // Is recursively calculated after loading the database
};