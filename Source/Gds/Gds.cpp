#define _CRT_SECURE_NO_WARNINGS

#include "Gds.h"
#include "Records.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static
double buffer_to_double(unsigned char* p)
{
	int i, sign, exp;
	double fraction;

	// The binary representation is with 7 bits pair the exponent and 56 bits pair
	// the mantissa:
	//
	// SEEEEEEE MMMMMMMM MMMMMMMM MMMMMMMM
	// MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

	// Denominators for the 7 fraction bytes
	uint64_t div[7] = {
		0x0100,
		0x010000,
		0x01000000,
		0x0100000000,
		0x010000000000,
		0x01000000000000,
		0x0100000000000000
	};

	sign = 1;
	exp = 0;
	if (p[0] > 127){
		sign = -1;
		exp = p[0] - 192;
	} else{
		exp = p[0] - 64;
	}

	fraction = 0.0;
	for (i = 0; i < 7; ++i){
		fraction += p[i + 1] / (double)div[i];
	}

	return (pow(16, exp) * sign * fraction);
}

static
void swap_big_endian(uint64_t* buf, uint64_t n)
{
	while (n > 0){
		uint64_t b = *buf;
		*buf++ = (b << 56) | ((b & 0x000000000000FF00) << 40) |
			((b & 0x0000000000FF0000) << 24) | ((b & 0x00000000FF000000) << 8) |
			((b & 0x000000FF00000000) >> 8) | ((b & 0x0000FF0000000000) >> 24) |
			((b & 0x00FF000000000000) >> 40) | (b >> 56);

		n--;
	};
}

static
void double_to_buffer(double value, uint8_t* buf)
{
	// Write a GDS 8 byte double to a 8 byte buffer in floating point format of GDSII
	// 8 bytes of memory need to be reserved for @buf

	memset(buf, 0, 8);

	if (value == 0) return;

	// The first byte with sign and exponent SEEEEEEE

	uint8_t left_byte = 0;
	if (value < 0){
		left_byte = 0x80;
		value = -value;
	}

	const double fexp = 0.25 * log2(value);
	double exponent = ceil(fexp);
	if (exponent == fexp) exponent++;
	left_byte += (uint8_t)(64 + exponent);

	const uint64_t mantissa = (uint64_t)(value * pow(16, 14 - exponent));

	union{
		uint8_t a[8];
		uint64_t b;
	} tmp = {0};

	tmp.b = ((uint64_t)left_byte << 56) | (mantissa & 0x00FFFFFFFFFFFFFF);

	swap_big_endian(&tmp.b, 8);

	for (int i = 0; i < 8; i++){
		buf[i] = tmp.a[i];
	}
}

static
int read_cells(gds_db* db, const wchar_t* file)
{
	FILE* fp;
	_wfopen_s(&fp, file, L"rb");
	if (!fp)
		return GDS_ERR_FILE_OPEN;

	enum ElemType{ EL_NONE = 0, EL_BOUNDARY, EL_PATH, EL_SREF, EL_AREF, EL_TEXT, EL_NODE, EL_BOX };

	// We keep track of path warnings given to avoid repeat
	bool pathtype1_warning_given = false;
	bool pathtype4_warning_given = false;

	// Pointers to the active cell and possible active elements)
	std::unique_ptr<gds_cell> active_cell = NULL;

	std::unique_ptr<gds_boundary> active_boundary = NULL;
	std::unique_ptr<gds_path> active_path = NULL;
	std::unique_ptr<gds_sref> active_sref = NULL;
	std::unique_ptr<gds_aref> active_aref = NULL;

	// Variable to track the type of element currently being read
	enum ElemType curElem = EL_NONE;

	// Variable to track if the ENDLIB record was read
	bool endlib = false;

	uint64_t bytes_read = 0; // Number of bytes read from file
	uint8_t header[4];

	while (endlib == false && fread(header, 1, 4, fp) == 4){
		uint16_t buf_size, record_len, record_type;

		bytes_read += 4;

		// First 2 bytes: record length; second 2 bytes record type and data type
		record_len = (header[0] << 8) | header[1];
		record_type = header[2] << 8 | header[3];

		if (record_len < 4)
			return GDS_ERR_RECORD_LENGTH;

		// The size of the payload
		buf_size = record_len - 4;

		// Read the data (buf_size == 0 means a zero payload record)
		unsigned char* buf = NULL;
		if (buf_size > 0){
			buf = (unsigned char*)malloc(buf_size * sizeof(*buf));
			bytes_read += fread(buf, 1, buf_size, fp);
		}

		// Handle the different GDS records
		switch (record_type){
			case HEADER:
			{
				db->version = buf[0] << 8 | buf[1];
				break;
			}
			case BGNLIB:
				break;
			case ENDLIB:
			{
				endlib = true;
				break;
			}
			case LIBNAME:
				break;
			case BGNSTR:
			{
				// Needs to have a prior ENDSTR (or the first BGNSTR in the file)
				if (active_cell != NULL)
					return GDS_ERR_ILLEGAL_BGNSTR;

				active_cell = std::make_unique<gds_cell>();

				// Ensure the pointer to the cell is registered already so no memory leaks when
				// an error is found

				break;
			}
			case ENDSTR:
			{
				// Needs to have a prior BGNSTR
				if (active_cell.get() == NULL)
					return GDS_ERR_ILLEGAL_ENDSTR;

				db->cell_list.push_back(std::move(active_cell));

				break;
			}
			case UNITS:
			{
				db->dbunit_in_uu = buffer_to_double(buf);
				db->dbunit_in_meter = buffer_to_double(buf + 8);

				if (db->dbunit_in_uu <= 0. || db->dbunit_in_meter <= 0)
					return GDS_ERR_DBU;

				break;
			}
			case STRNAME:
			{
				if (active_cell == NULL || buf_size > GDS_MAX_CELL_NAME)
					return GDS_ERR_ILLEGAL_STRNAME;

				strncpy(active_cell->name, (char*)buf, buf_size);
				active_cell->name[buf_size] = '\0';

				break;
			}
			case BOUNDARY:
			{
				if (curElem != EL_NONE || active_cell == NULL)
					return GDS_ERR_ILLEGAL_BOUNDARY;

				active_boundary = std::make_unique<gds_boundary>();

				curElem = EL_BOUNDARY;

				break;
			}
			case PATH:
			{
				if (curElem != EL_NONE || active_cell == NULL)
					return GDS_ERR_ILLEGAL_PATH;

				active_path = std::make_unique<gds_path>();;

				curElem = EL_PATH;

				break;
			}
			case SREF:
			{
				if (curElem != EL_NONE || active_cell == NULL)
					return GDS_ERR_ILLEGAL_SREF;

				active_sref = std::make_unique<gds_sref>();;

				gds_sref* tmp = active_sref.get();
				tmp->mag = 1.0f;

				curElem = EL_SREF;

				break;
			}
			case AREF:
			{
				if (curElem != EL_NONE || active_cell == NULL)
					return GDS_ERR_ILLEGAL_AREF;

				active_aref = std::make_unique<gds_aref>();;

				gds_aref* tmp = active_aref.get();
				tmp->mag = 1.0f;

				curElem = EL_AREF;

				break;
			}
			case TEXT:
				curElem = EL_TEXT;
				break;
			case NODE:
				curElem = EL_NODE;
				break;
			case BOX:
				curElem = EL_BOX;
				break;
			case ENDEL:
			{
				if (active_cell == NULL)
					return GDS_ERR_ILLEGAL_ENDEL;

				switch (curElem){
					case EL_BOUNDARY:
					{
						// Calculate the boundary box
						gds_boundary* b = active_boundary.get();
						bbox_init(&b->bbox);
						bbox_fit_points(&b->bbox, &b->pairs[0], (int)b->pairs.size());

						active_cell->boundaries.push_back(std::move(active_boundary));

						break;
					}
					case EL_PATH:
					{
						// Expand the path

						gds_path* p = active_path.get();

						// Resize the extended pairs element of peth
						p->epairs.resize(2 * p->pairs.size() + 1);

						int result = gds_expand_path(&p->epairs[0], &p->pairs[0], (int)p->pairs.size(),
							p->width, p->pathtype);

						if (result == EXIT_FAILURE)
							return GDS_ERR_PATH_EXPANSION;

						bbox_init(&p->bbox);
						bbox_fit_points(&p->bbox, &p->epairs[0], p->epairs.size());

						active_cell->paths.push_back(std::move(active_path));

						break;
					}
					case EL_SREF:
					{
						active_cell->srefs.push_back(std::move(active_sref));

						break;
					}
					case EL_AREF:
					{
						active_cell->arefs.push_back(std::move(active_aref));

						break;
					}
				}

				curElem = EL_NONE;

				break;
			}
			case SNAME: // SREF, AREF
			{
				if (active_cell == NULL || buf_size > GDS_MAX_CELL_NAME)
					return GDS_ERR_ILLEGAL_STRNAME;

				switch (curElem){
					case EL_SREF:
					{
						if (active_sref.get() == NULL)
							return GDS_ERR_ILLEGAL_STRNAME;

						strncpy(active_sref.get()->sname, (char*)buf, buf_size);
						active_sref.get()->sname[buf_size] = '\0';
						break;
					}
					case EL_AREF:
					{
						if (active_aref.get() == NULL)
							return GDS_ERR_ILLEGAL_STRNAME;

						strncpy(active_aref.get()->sname, (char*)buf, buf_size);
						active_aref.get()->sname[buf_size] = '\0';
						break;
					}
					default:
						return GDS_ERR_ILLEGAL_SNAME;
				}

				break;
			}
			case COLROW: // AREF
			{
				if (curElem != EL_AREF || active_aref.get() == NULL)
					return GDS_ERR_ILLEGAL_COLROW;

				active_aref.get()->ncols = buf[0] << 8 | buf[1];
				active_aref.get()->nrows = buf[2] << 8 | buf[3];

				break;
			}
			case PATHTYPE:
			{
				if (curElem != EL_PATH || active_path.get() == NULL)
					return GDS_ERR_ILLEGAL_PATHTYPE;

				active_path.get()->pathtype = buf[0] << 8 | buf[1];

				// Only path types 0 and 2 are supported. Type 1 and 4 are converted to type 2 with
				// a warning.

				if (!pathtype1_warning_given && active_path.get()->pathtype == 1){
					printf("WARNING: path type 1 (round ended) will be converted to type 2\n");
					active_path.get()->pathtype = 2;
					pathtype1_warning_given = true;
				}

				if (!pathtype4_warning_given && active_path.get()->pathtype == 4){
					printf("WARNING: path type 4 (var length) will be converted to type 2\n");
					active_path.get()->pathtype = 2;
					pathtype4_warning_given = true;
				}

				break;
			}
			case STRANS: // SREF, AREF, TEXT
			{
				switch (curElem){
					case EL_SREF:
						active_sref.get()->strans = buf[0] << 8 | buf[1];
						break;
					case EL_AREF:
						active_aref.get()->strans = buf[0] << 8 | buf[1];
						break;
				}
				break;
			}
			case ANGLE: // SREF, AREF, TEXT
			{
				switch (curElem){
					case EL_SREF:
					{
						active_sref.get()->angle = (float)(M_PI * buffer_to_double(buf) / 180.0);
						break;
					}
					case EL_AREF:
					{
						active_aref.get()->angle = (float)(M_PI * buffer_to_double(buf) / 180.0);
						break;
					}
				}
				break;
			}
			case MAG: // SREF, AREF, TEXT
			{
				switch (curElem){
					case EL_SREF:
						active_sref.get()->mag = (float)buffer_to_double(buf);
						break;
					case EL_AREF:
						active_aref.get()->mag = (float)buffer_to_double(buf);
						break;
				}
				break;
			}
			case XY:
			{
				// Number of pairs
				int count = buf_size / 8;

				switch (curElem){
					case EL_BOUNDARY:
					{
						active_boundary->pairs.resize(count);
						break;
					}
					case EL_PATH:
					{
						active_path->pairs.resize(count);
						break;
					}
					case EL_SREF:
						assert(count == 1);
						break;
					case EL_AREF:
						assert(count == 3);
						break;
				}

				for (int n = 0; n < count; n++){
					int i = 8 * n;

					int x = buf[i] << 24 | buf[i + 1] << 16 | buf[i + 2] << 8 | buf[i + 3];
					int y = buf[i + 4] << 24 | buf[i + 5] << 16 | buf[i + 6] << 8 | buf[i + 7];

					switch (curElem){
						case EL_BOUNDARY:
							active_boundary->pairs[n] = {x, y};
							break;
						case EL_PATH:
							active_path->pairs[n] = {x, y};
							break;
						case EL_SREF:
							active_sref->origin = {x, y};
							break;
						case EL_AREF:
							active_aref->vectors[n] = {x, y};
							break;
					}
				}
				break;
			}
			case LAYER: // BOUNDARY, PATH, TEXT, NODE, BOX
			{
				switch (curElem){
					case EL_BOUNDARY:
						active_boundary->layer = buf[0] << 8 | buf[1];
						break;
					case EL_PATH:
						active_path->layer = buf[0] << 8 | buf[1];
						break;
				}
				break;
			}
			case WIDTH: // PATH, TEXT
				if (curElem == EL_PATH)
					active_path->width = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
				break;
			case DATATYPE:
				break;
			case TEXTNODE:
				break;
			case TEXTTYPE:
				break;
			case PRESENTATION:
				break;
			case STRING:
				break;
			case REFLIBS:
				break;
			case FONTS:
				break;
			case ATTRTABLE:
				break;
			case ELFLAGS:
				break;
			case PROPATTR:
				break;
			case PROPVALUE:
				break;
			case BOXTYPE:
				break;
			case PLEX:
				break;
			case BGNEXTN:
				break;
			case ENDEXTN:
				break;
			case FORMAT:
				break;
			default:
				break;
		}

		free(buf);
	}

	fclose(fp);

	//
	// Make sure all referenced cell names exist and assign cell pointers
	//

	for (auto&& cell : db->cell_list){
		//gds_cell* cell = db->cell_list[i];

		for (auto&& sref : cell->srefs){
			// Continue if this reference already has the cell pointer determined
			if (sref->cell != NULL)
				continue;

			sref->cell = (gds_cell*)find_cell(db, sref->sname);

			if (sref->cell == NULL)
				return GDS_ERR_CELL_NAME_NOT_FOUND;
		}

		for (auto&& aref : cell->arefs){

			// Continue if this reference already has the cell pointer determined
			if (aref->cell)
				continue;

			aref->cell = (gds_cell*)find_cell(db, aref->sname);

			if (!aref->cell)
				return GDS_ERR_CELL_NAME_NOT_FOUND;
		}
	}

	return GDS_ERR_SUCCESS;
}


gds_db::gds_db()
{
	int a{};
}

int gds_db::read(const wchar_t* file)
{
	int error = read_cells(this, file);

	// Determine the size of each cell
	gds_cell_sizes(this);

	return error;
}

gds_db::~gds_db()
{

}
