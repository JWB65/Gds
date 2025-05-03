A simple library for reading in a GDSII layout database file (*.gds) commonly used in the semiconductor, MEMS, and TFT industries. After reading in the database, polygons can be extracted from it
for further manipulation by the library user.

# Features

* Read-in a GDSII database which basically is a list of cells (also called structures) each with several elements like polygons and references to other cells.
* Extract polygons from a cell in the database to a set of polygons.
* Write the polygons to a newly created GDSII file.

# How to add the library to your project

No dependencies except the standard library. Copy the folder named Gds with source files to your project and include the file `Gds.h` therein in your projects.

# Structures



# Example use

* An example of its use is given in the `Test.cpp` in the Test folder with further information on the use of each function.

Questions: janwillembos@yahoo.com
