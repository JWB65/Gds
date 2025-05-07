#include "Gds.h"

#include <inttypes.h>
#include <stdio.h>

static gds_bbox
cell_sizes_recurse(gds_cell* cell, gds_transform transform, unsigned int level)
{
	if (cell->bbox.has_value())
		return cell->bbox.value().transform(transform, false);

	// We start with an empty bounding box at the origin and start filling it recursively with all
	// boundary elements, paths, and reference cells

	gds_bbox bbox_cell(0, 0, 0, 0);

	for (gds_boundary& b : cell->boundaries) {
		bbox_cell.fit_bbox(b.bbox);
	}

	for (gds_path& p : cell->paths) {
		bbox_cell.fit_bbox(p.bbox);
	}

	// Transform the resulting bounding box with the accumulated transformation
	bbox_cell.transform(transform, false);

	// Update the bounding box to fit all gds_sref elements
	for (gds_sref& sref : cell->srefs) {

		gds_transform acc;
		acc.translation = transform_pair(sref.origin, &transform, false);
		acc.magnification = transform.magnification * sref.mag;
		acc.angle = transform.angle + sref.angle;
		acc.mirror = transform.mirror ^ (sref.strans & 0x8000);

		// Recurse into the reference cell
		gds_bbox tmp = cell_sizes_recurse(sref.cell, acc, level + 1);

		// adjust the bounding box
		bbox_cell.fit_bbox(tmp);
	}

	for (gds_aref& aref : cell->arefs) {

		double v_col_x, v_col_y, v_row_x, v_row_y;

		int64_t x1 = aref.vectors[0].x;
		int64_t y1 = aref.vectors[0].y;
		int64_t x2 = aref.vectors[1].x;
		int64_t y2 = aref.vectors[1].y;
		int64_t x3 = aref.vectors[2].x;
		int64_t y3 = aref.vectors[2].y;

		// (v_col_x, v_col_y) vector pair column direction
		v_col_x = ((double)(x2 - x1)) / aref.ncols;
		v_col_y = ((double)(y2 - y1)) / aref.ncols;

		// (v_row_x, v_row_y) vector pair row direction
		v_row_x = ((double)(x3 - x1)) / aref.nrows;
		v_row_y = ((double)(y3 - y1)) / aref.nrows;

		// Loop through the array and recurse each
		for (int c = 0; c < aref.ncols; c++) {
			for (int r = 0; r < aref.nrows; r++) {
				// Position of the sub structure cell being referenced

				int64_t x_ref = (int64_t)(x1 + c * v_col_x + r * v_row_x);
				int64_t y_ref = (int64_t)(y1 + c * v_col_y + r * v_row_y);

				gds_transform acc;
				acc.translation = transform_pair({x_ref, y_ref}, &transform, false);
				acc.magnification = transform.magnification * aref.mag;
				acc.angle = transform.angle + aref.angle;
				acc.mirror = transform.mirror ^ (aref.strans & 0x8000);

				// Recurse into the reference cell
				gds_bbox tmp = cell_sizes_recurse(aref.cell, acc, level + 1);

				// adjust the bounding box
				bbox_cell.fit_bbox(tmp);
			}
		}
	}

	// Find and store the bounding box of the structure without transformation (by doing the inverse) 
	cell->bbox = bbox_cell.transform(transform, true);

	return bbox_cell;
}

void gds_cell_sizes(gds_db* db)
{
	// Calculate cell bounding boxes and output to console

	printf("\nAll cells in database with width and height in database units:\n");

	for (gds_cell& cell : db->cell_list) {

		gds_transform transform;
		transform.translation = {0, 0};
		transform.magnification = 1.f;
		transform.angle = 0.f;
		transform.mirror = 0x0000;

		gds_bbox box = cell_sizes_recurse(&cell, transform, 0);

		printf("%s: %lld by %lld\n", cell.name, box.xmax - box.ymin, box.ymax - box.ymin);
	}
}
