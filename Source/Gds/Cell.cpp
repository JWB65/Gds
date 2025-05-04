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
		delete sref;
	}
	delete srefs;

	for (gds_aref* aref : *arefs) {
		delete aref;
	}
	delete arefs;

	for (gds_boundary* elem : *boundaries) {
		delete elem->pairs;
		delete elem;
	}
	delete boundaries;

	for (gds_path* elem : *paths) {
		//gds_path* elem = (gds_path*) cell->paths[j];
		delete elem->pairs;
		delete elem->epairs;
		delete elem;
	}
	delete paths;
}
