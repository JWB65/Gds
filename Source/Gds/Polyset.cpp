#include "Polyset.h"

#include <stdlib.h>


int gds_write(const wchar_t* dest, gds_polyset* pset, double dbunit_size_uu, double dbunit_size_in_m);


gds_polygon::gds_polygon(gds_pair* pairs, int npairs, gds_bbox box, uint16_t layer)
{
	this->pairs = new gds_pair[npairs];
	this->npairs = npairs;

	for (int i = 0; i < npairs; i++){
		this->pairs[i] = pairs[i];
	}

	this->bbox = box;
	this->layer = layer;
}

gds_polygon::~gds_polygon()
{
	delete[] pairs;
}


gds_polyset::gds_polyset()
{
	set = new std::vector<gds_polygon*>;

};

gds_polyset::~gds_polyset()
{
	for (gds_polygon* elem : *set){
		delete elem;
	}
	delete set;
}

void gds_polyset::add(gds_polygon* p)
{
	set->push_back(p);
}

size_t gds_polyset::size()
{
	return set->size();
}

void gds_polyset::clear()
{
	set->clear();
}

void gds_polyset::print()
{
	printf("\nPrinting polygon set:\n");

	if (set->size() == 0)
		printf("--> List is empty\n");

	for (gds_polygon* poly : *set){
		printf("\nLayer %d polygon with %d vertices:\n", poly->layer, poly->npairs - 1);
		for (int j = 0; j < poly->npairs; j++){
			gds_pair pair = poly->pairs[j];
			printf("\t(%lld, %lld)\n", pair.x, pair.y);
		}
	}
}

int gds_polyset::write(const wchar_t* dest, double dbunit_size_uu, double dbunit_size_in_m)
{
	return gds_write(dest, this, dbunit_size_uu, dbunit_size_in_m);
}
