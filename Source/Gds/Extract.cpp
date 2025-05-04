#include "gds.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define GDS_MAX_POLYS 1000000

typedef struct ExtractionInfo
{
	gds_polyset* pset;
	gds_bbox target;
	int64_t resolution;
	int64_t nskipped;
	char* error;
} ExtractionInfo;

static
void add_poly(gds_polyset* pset, gds_pair* pairs, int npairs, uint16_t layer, gds_bbox* box, gds_transform* tra)
{
	//gds_polygon* poly = (gds_polygon *) malloc(sizeof(gds_polygon));

	//gds_pair* pairs = (gds_pair*)malloc(npairs * sizeof(gds_pair));
	gds_pair* transformed_pairs = new gds_pair[npairs];
	transform_pairs(transformed_pairs, pairs, npairs, tra, false);

	gds_polygon* poly = new gds_polygon(transformed_pairs, npairs, *box, layer);

	pset->add(poly);

	delete[] transformed_pairs;
}

static
void extract(ExtractionInfo* info, gds_cell* cell, gds_transform transform, int level)
{
	for (gds_boundary* b : *cell->boundaries)
	{
		gds_bbox b_bbox = bbox_transform(&b->bbox, &transform, false);

		if (bbox_check_overlap(&b_bbox, &info->target))
		{
			// The boundary element after transformation overlaps with the target bounding box

			// Check if its size is larger than the minimum (resolution)
			int64_t box_size = bbox_size(&b_bbox);

			if (box_size < info->resolution)
			{
				info->nskipped++;
			} else
			{
				add_poly(info->pset, b->pairs, b->npairs, b->layer, &b_bbox, &transform);
			}

			// Check if the number of polygons is overflowing
			if (info->pset->size() >= GDS_MAX_POLYS)
			{
				info->error = (char*)"Reached maximum allowed number of polygons";
				return;
			}
		}
	}

	for (gds_path* p : *cell->paths)
	{
		//gds_path* p = (gds_path*)cell->paths[i];
		gds_bbox bbox = bbox_transform(&p->bbox, &transform, false);

		if (bbox_check_overlap(&bbox, &info->target))
		{
			// The boundary element after transformation overlaps with the target bounding box

			// Check if its size is larger than the minimum (resolution)
			int64_t box_size = bbox_size(&bbox);

			if (box_size < info->resolution)
			{
				info->nskipped++;
			} else
			{
				add_poly(info->pset, p->pairs, p->npairs, p->layer, &bbox, &transform);
			}

			// Check if the number of polygons is overflowing
			if (info->pset->size() >= GDS_MAX_POLYS)
			{
				info->error = (char *) "Reached maximum allowed number of polygons";
				return;
			}
		}
	}

	// Scan through the SREF elements
	for (gds_sref* sref : *cell->srefs)
	{
		gds_transform acc;
		acc.translation = transform_pair(sref->origin, &transform, false);
		acc.magnification = transform.magnification * sref->mag;
		acc.angle = transform.angle + sref->angle;
		acc.mirror = transform.mirror ^ (sref->strans & 0x8000);

		// Transform the bounding box of the SREF element
		gds_bbox sref_box = bbox_transform(&sref->cell->bbox, &acc, false);

		// Recurse further only if the sref bounding overlaps with the target bounding box
		if (bbox_check_overlap(&sref_box, &info->target))
		{
			extract(info, sref->cell, acc, level + 1);

			if (info->error != NULL)
				return; // Collapse recursion
		}
	}

	// Scan through the AREF elements
	for (gds_aref* aref : *cell->arefs)
	{
		double v_col_x, v_col_y, v_row_x, v_row_y;

		int64_t x1 = aref->vectors[0].x;
		int64_t y1 = aref->vectors[0].y;
		int64_t x2 = aref->vectors[1].x;
		int64_t y2 = aref->vectors[1].y;
		int64_t x3 = aref->vectors[2].x;
		int64_t y3 = aref->vectors[2].y;

		// (v_col_x, v_col_y) vector pair column direction
		v_col_x = ((double)(x2 - x1)) / aref->ncols;
		v_col_y = ((double)(y2 - y1)) / aref->ncols;

		// (v_row_x, v_row_y) vector pair row direction
		v_row_x = ((double)(x3 - x1)) / aref->nrows;
		v_row_y = ((double)(y3 - y1)) / aref->nrows;

		// Loop through the array and recurse each
		for (int c = 0; c < aref->ncols; c++)
		{
			for (int r = 0; r < aref->nrows; r++)
			{
				// Position of the sub structure cell being referenced

				int64_t x_ref = (int64_t)(x1 + c * v_col_x + r * v_row_x);
				int64_t y_ref = (int64_t)(y1 + c * v_col_y + r * v_row_y);

				gds_transform acc;
				acc.translation = transform_pair({ x_ref, y_ref }, & transform, false);
				acc.magnification = transform.magnification * aref->mag;
				acc.angle = transform.angle + aref->angle;
				acc.mirror = transform.mirror ^ (aref->strans & 0x8000);

				// Transform the bounding box of the aref element
				gds_bbox aref_box = bbox_transform(&aref->cell->bbox, &acc, false);

				// Recurse further only if the aref bounding overlaps with the target bounding box
				if (bbox_check_overlap(&aref_box, &info->target))
				{
					extract(info, aref->cell, acc, level + 1);

					if (info->error != NULL)
						return; // Collapse recursion
				}
			}
		}
	}
}

int gds_extract(gds_db* db, const char* cell_name, gds_bbox target, int64_t resolution, gds_polyset* pset,
	int64_t* nskipped)
{
	// Find the pointer to the structure to expand
	gds_cell* top = find_cell(db, cell_name);

	printf("\nExtracting polygons from cell %s:\n", cell_name);

	if (!top)
	{
		printf("\n--> Cell name not found\n");
		return GDS_ERR_CELL_NAME_NOT_FOUND;
	}

	ExtractionInfo info;

	info.pset = pset;
	info.nskipped = 0;
	info.error = NULL;

	//double dbunit_in_um = 1E6 * db->dbunit_in_meter;

	// Convert the target box from micron to data base units
	info.target = target;

	// Convert the target resolution from micron to data base units
	info.resolution = resolution;

	// Initial transformation

	gds_transform transfrom;
	transfrom.translation = {0, 0};
	transfrom.magnification = 1.f;
	transfrom.angle = 0.f;
	transfrom.mirror = 0x0000;

	extract(&info, top, transfrom, 1);

	if (info.error != NULL)
	{
		printf("--> %s\n", info.error);
		return GDS_ERR_MAX_POLYS;
	}

	if (info.pset->size() == 0)
	{
		printf("--> No polygons found\n");
		return GDS_ERR_NO_POLYS_FOUND;
	} else
	{
		printf("--> Found %lld polygons\n", info.pset->size());
	}

	*nskipped = info.nskipped;

	return GDS_ERR_SUCCESS;
}
