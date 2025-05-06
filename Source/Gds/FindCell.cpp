#include "Gds.h"

gds_cell* find_cell(gds_db* db, const char* name)
{
	// Returns pointer cell with name @name in gds database @db or NULL if not found

	for (gds_cell& cell : db->cell_list) {
		if (strcmp(cell.name, name) == 0)
			return &cell;
	}

	return NULL;
}