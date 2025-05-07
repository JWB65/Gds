#include "../Gds/Gds.h"

#include <stdio.h>
#include <stdlib.h>
#include <tuple>

int main()
{
	//
	// Example use of the Gds library
	//

	// Name of the GDS database file (*.gds) to load.
	wchar_t name[] = L"C:\\LOCAL\\TEST\\Test.gds";

	// Initialize an empty database object
	gds_db db;

	// Read the database
	int result = db.read(name);

	if (result != GDS_ERR_SUCCESS) {
		printf("\nError %d found while opening GDS database\n", result);
		return 0;
	}

	//
	// Extract polygons from a rectangular region in a cell in the GDSII data base
	//
	
	// Obtain the target rectangle bounding box in database units
	double um_per_db = 1E6 * db.m_per_dbu;
	int64_t x_min = (int64_t)(-23441 / um_per_db);
	int64_t y_min = (int64_t)(17292 / um_per_db);
	int64_t x_max = (int64_t)((-23441 + 48) / um_per_db);
	int64_t y_max = (int64_t)((17292 + 48) / um_per_db);
	gds_bbox target(x_min, y_min, x_max, y_max);

	// The resolution (min polygon size to include) also has to be converted in database units
	int64_t resolution = (int64_t) (1.5 / um_per_db);

	// Empty set of polygons to receive the extracted polygons
	gds_polyset pset;

	// Do the polygon extraction
	auto [eresult, nskipped] = gds_extract(&db, "0_PM02_Mask", target, resolution, &pset);

	// The first element of the return tuple is an error code
	if (eresult != GDS_ERR_SUCCESS) {
		printf("--> Error %d found while extracting polygons\n", result);
		return 0;
	}

	//
	// Write the polygons to a new GDS file
	//

	if (pset.size() > 0)
		pset.write(L"c:\\LOCAL\\TEST\\test_out.gds", db.uu_per_dbu, db.m_per_dbu);

	return 0;
}