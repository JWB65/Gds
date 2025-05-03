A simple library for reading in a GDSII layout database file (*.gds) commonly used in the semiconductor, MEMS, and TFT industries. After reading in the database, polygons can be extracted from it
for further manipulation by the library user.

# Features

* Read-in a GDSII database which basically is a list of cells (also called structures) each with several elements like polygons and references to other cells.
* Extract polygons from a cell in the database to a set of polygons.
* Write the polygons to a newly created GDSII file.

# How to add the library to your project

No dependencies except the standard library. Copy the folder named Gds with source files to your project and include the file `Gds.h` therein in your projects.

# Structures

The basis structure is the `gds_db` which holds information on a GDSII database that is read-in from a file. Polygons can be extracted from it to a `gds_polyset` structure defined as

`typedef std::vector<gds_polygon*> gds_polyset;`

which consists of a `std::vector` of pointers to `gds_polygon` structures. After usage the `gds_polyset` must be cleared by `gds_polyset_clear(pset)`.

Each `gds_polygon` structure consists of a array of pairs with coordinates of the polygon vertices in GDSII database units:

```
class gds_polygon {
public:

	// The constructor requires an already memory allocated array of type gds_pair
	gds_polygon(gds_pair* pairs, int npairs, gds_bbox box, uint16_t layer);

	// The destructor takes care of deleting the allocated memory in member pairs
	~gds_polygon();

	gds_pair* pairs; // Coordinates of vertices in database units
	int npairs; // Number of coordinates (note GDSII polygons are closed polygons)

	uint16_t layer; // Layer in the GDSII database to which the polygon belongs

	gds_bbox bbox; // Bounding box of the polygon
};
```

# Example use

* An example of its use is given in the `Test.cpp` in the Test folder with further information on the use of each function.

Questions: janwillembos@yahoo.com
