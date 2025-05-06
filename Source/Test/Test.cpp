#include "../Gds/Gds.h"

#include <stdio.h>
#include <stdlib.h>
#include <tuple>

int main()
{
	//
	// Example use of the Gds library
	//

	// Name of the GDS database file (*.gds) to load. Make sure to change.
	wchar_t name[] = L"C:\\LOCAL\\TEST\\Test.gds";

	// Initialize an empty database object
	gds_db db;

	// Read the file
	int result = db.read(name);

	if (result != GDS_ERR_SUCCESS) {
		printf("\nError %d found while opening GDS database\n", result);
		return 0;
	}

	//
	// Extract polygons from a region in a cell in the GDSII data base
	//
	// We need to convert the coordinates of the region from um to database units by
	//		x_db = x_um / factor   [where]   factor = 1E6 * db->dbunit_in_meter
	//		(same for y)

	// The target rectangle. Only polygons that overlap with this rectange will be included.

	double factor = 1E6 * db.dbunit_in_meter;

	int64_t x_min = (int64_t)(-23441 / factor);
	int64_t y_min = (int64_t)(17292 / factor);
	int64_t x_max = (int64_t)((-23441 + 48) / factor);
	int64_t y_max = (int64_t)((17292 + 48) / factor);

	gds_bbox target = {x_min, y_min, x_max, y_max};

	// The resolution (min polygon size to include) also has to be converted in database units
	int64_t resolution = (int64_t) (1.5 / factor);

	// Empty set of polygons to receive the extracted polygons
	gds_polyset pset;

	// Do the polygon extraction
	auto [eresult, nskipped] = gds_extract(&db, "0_PM02_Mask", target, resolution, &pset);

	if (eresult != GDS_ERR_SUCCESS) {
		printf("--> Error %d found while extracting polygons\n", result);
		return 0;
	}

	//
	// Write the polygons to a new GDS file
	//

	if (pset.size() > 0)
		pset.write(L"c:\\LOCAL\\TEST\\test_out.gds", db.dbunit_in_uu, db.dbunit_in_meter);

	return 0;
}