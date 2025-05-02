#include "../Gds/Gds.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
	//
	// Construct a GDSII database object
	//

	wchar_t name[] = L"C:\\LOCAL\\TEST\\Test.gds";

	gds_db* db = new gds_db();

	// Read the file
	int result = db->read(name);

	if (result != GDS_ERR_SUCCESS) {
		printf("\nError %d found while opening GDS database\n", result);

		delete db;
		db = NULL;

		return 1;
	}

	//
	// Extract polygons from a region in a cell in the GDSII data base
	//
	// We need to convert the coordinates of the region from um to database units by
	//		x_db = x_um / factor   [where]   factor = 1E6 * db->dbunit_in_meter
	//		(same for y)

	// The target rectangle. Only polygons that overlap with this rectange will be included.

	double factor = 1E6 * db->dbunit_in_meter;

	int64_t x_min = (int64_t)(-23441 / factor);
	int64_t y_min = (int64_t)(17292 / factor);
	int64_t x_max = (int64_t)((-23441 + 48) / factor);
	int64_t y_max = (int64_t)((17292 + 48) / factor);

	gds_bbox target = {x_min, y_min, x_max, y_max};

	// The resolution (min polygon size to include) also has to be converted in database units
	int64_t resolution = (int64_t) (1.5 / factor);

	// The polygon set to receive the output polygons after extraction
	gds_polyset* pset = new gds_polyset;

	// The number of polygons skipped during extraction because their size < resolution
	int64_t nskipped = 0;

	// Do the polygon extraction
	result = gds_extract(db, "0_PM02_Mask", target, resolution, pset, &nskipped);

	if (result != GDS_ERR_SUCCESS) {
		printf("--> Error %d found while extracting polygons\n", result);

		gds_polyset_clear(pset);
		delete pset;
		pset = NULL;
	}

	//
	// Write the polygons to a new GDS file
	//

	if (pset != NULL)
	{
		gds_write(L"c:\\LOCAL\\TEST\\test_out.gds", pset, db->dbunit_in_uu, db->dbunit_in_meter);
		gds_polyset_clear(pset);
		delete pset;
		pset = NULL;
	}

	// Delete the database
	delete db;
	db = NULL;

	return 1;
}