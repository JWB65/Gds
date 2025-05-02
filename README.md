# GDSII

A simple library for reading and extracting polygons from a GDSII layout database used in the semiconductor, MEMS, and TFT industries.

# Features

* Read-in a GDSII database which basically is a list of cells (also called structures) each with several elements like polygons and references to other cells.
* Extract polygons from a GDSII cell. The user of the library can use this pointer list for further programming work.
* Write the polygons to a newly created GDSII file.

# How to add the library to your project

No dependencies except the standard library. Just move all files in the Gds folder and List folder to your project and include
```
gds.h
list.h
```
in your project. A list is a pretty standard dynamic list of void pointers which can use to store general objects. In particular here it is used to store the polygons
extracted from a cell in a GDSII database.

# Instructions to use

* An example of its use is given in the `test.c` in the Test folder with further information on the use of each function.

* Create a GDSII database with ```gds_db* db = gds_new(name, &error);```.

* Read in the polygons of a given cell into a pointer list by ```gds_extract(db, cell_name, target, 1.5, pset, &nskipped);```. Only polygons that overlap with bounding
  box `target` are included. Also, in this example, the polygons need to be larger than the resolution `1.5` micron. The number of polygons that are skipped because their size
  is below `1.5` micron are placed in `nskipped`.
  The polygons are stored in the list pointed to by `pset`.

* If desired, create a new GDSII file from the extracted polygons with ```gds_write(L"c:\\foo.gds", pset, db->dbunit_in_uu, db->dbunit_in_meter);```.

# The polygon structure

The polygon structure `gds_polygon` is just an array of pairs with an integer specifying the layer number in the GDSII database.

```
typedef struct gds_pair
{
	int64_t x, y;
} gds_pair;

typedef struct gds_polygon
{
	gds_pair* pairs; // Coordinates of vertices in database units
	int npairs; // Number of coordinates (note GDSII polygons are closed polygons)

	uint16_t layer; // Layer in the GDSII database to which the polygon belongs

	gds_bbox bbox; // Bounding box of the polygon in database units
} gds_polygon;
```
The `layer` member of the structure identifies the GDS layer number the polygon belongs to.

Questions: janwillembos@yahoo.com
