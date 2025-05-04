#pragma once

#include "BBox.h"
#include "Cell.h"
#include "Errors.h" // Error codes for the database constructor and poly extraction
#include "Polyset.h"

#include <stdbool.h>
#include <stdint.h>

#include <vector>

#define GDS_MAX_CELL_NAME 32

class gds_db {
public:
	uint16_t version{};

	// The contents of the gds UNIT records
	double dbunit_in_uu{}, dbunit_in_meter{};

	std::vector<gds_cell*> cell_list;

	gds_db();
	~gds_db();

	/*
		Construct a gds_db structure from a file. A pointer to int needs to be provided for a possible
		error code upon return.

		@file: file name of the GDS file to be loaded
		@error: pointer to int which will be filled with 0 (success) or an error code
		@return: A pointer to the gds_db if succesful or NULL if loading failed
	*/
	int read(const wchar_t* file);
};

// Defined in gds_expand_path.c
int gds_expand_path(gds_pair* out, const gds_pair* in, int npairs_in, uint32_t width, uint16_t pathtype);

// Defined in CellSizes.c
void gds_cell_sizes(gds_db* db);

// Find the pointer to cell with name @sname
gds_cell* find_cell(gds_db* db, const char* name);

/*
	Extract polygons from a region (given by @target) of a cell in a GDSII database

	@db: pointer to the gds_db structure previously loaded from a GDSII database file
	@cellname: name of the cell in @db to extract the polygons from
	@target: boundings box of the target area in database units
	@resolution: polygon sized smaller than @res (in database units) are ignored
	@pset: pointer to list of polygons
	@return: error code (in case of error)
 */
int gds_extract(gds_db* db, const char* cell_name, gds_bbox target, int64_t resolution,
	gds_polyset* pset, int64_t* nskipped);


