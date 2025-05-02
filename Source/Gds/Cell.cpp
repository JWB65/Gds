#include "Cell.h"

#include "BBox.h"

#include <limits.h>
#include <stdlib.h>

gds_cell::gds_cell()
{
	name[0] = 0;

	bbox_init(&bbox);

	initialized = false;

	srefs = new std::vector<gds_sref*>;
	arefs = new std::vector<gds_aref*>;
	boundaries = new std::vector<gds_boundary*>;
	paths = new std::vector<gds_path*>;

}

gds_cell::~gds_cell()
{
	for (gds_sref* sref : *srefs) {
		free(sref);
	}
	delete srefs;

	for (gds_aref* aref : *arefs) {
		free(aref);
	}
	delete arefs;

	for (gds_boundary* elem : *boundaries) {
		free(elem->pairs);
		free(elem);
	}
	delete boundaries;

	for (gds_path* elem : *paths) {
		//gds_path* elem = (gds_path*) cell->paths[j];
		free(elem->pairs);
		free(elem->epairs);
		free(elem);
	}
	delete paths;
}
