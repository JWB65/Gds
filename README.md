A simple library for reading and extracting polygons from a GDSII layout database used in the semiconductor, MEMS, and TFT industries.

# Features

* Read-in a GDSII database which basically is a list of cells (also called structures) each with several elements like polygons and references to other cells.
* Extract polygons from a cell in teh database.
* Write the polygons to a newly created GDSII file.

# How to add the library to your project

No dependencies except the standard library. Just copy the folder Gds with source files to your project and include the file `Gds.h` in source files using this library.

# Example use

* An example of its use is given in the `Test.cpp` in the Test folder with further information on the use of each function.

Questions: janwillembos@yahoo.com
