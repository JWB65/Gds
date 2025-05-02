#include "Polyset.h"

#include <stdlib.h>

gds_polygon::gds_polygon(gds_pair* pairs, int npairs, gds_bbox box, uint16_t layer)
{
	this->pairs = pairs; // Is allocated by the caller and needs to be destructed in the destructor
	this->npairs = npairs;
	this->bbox = box;
	this->layer = layer;
}

gds_polygon::~gds_polygon()
{
	delete[] pairs;
}

void gds_polyset_clear(gds_polyset* pset)
{
	for (gds_polygon* p : *pset) {
		delete p;
	}
}


void gds_print_polyset(gds_polyset* pset)
{
	printf("\nPrinting polygon set:\n");

	if (pset->size() == 0)
		printf("--> List is empty\n");

	for (gds_polygon* poly : *pset)
	{
		printf("\nLayer %d polygon with %d vertices:\n", poly->layer, poly->npairs - 1);
		for (int j = 0; j < poly->npairs; j++)
		{
			gds_pair pair = poly->pairs[j];
			printf("\t(%lld, %lld)\n", pair.x, pair.y);
		}
	}
}