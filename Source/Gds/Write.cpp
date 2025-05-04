#include "Gds.h"

#include "Records.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void swap_big_endian(uint64_t* buf, uint64_t n)
{
	while (n > 0)
	{
		uint64_t b = *buf;

		*buf++ = (b << 56) | ((b & 0x000000000000FF00) << 40) |
			((b & 0x0000000000FF0000) << 24) | ((b & 0x00000000FF000000) << 8) |
			((b & 0x000000FF00000000) >> 8) | ((b & 0x0000FF0000000000) >> 24) |
			((b & 0x00FF000000000000) >> 40) | (b >> 56);

		n--;
	}
}

static void double_to_buffer(double value, uint8_t* buf)
{
	/*
		Writes a double to a 8 byte buffer in floating point format of GDSII
		8 bytes of memory need to be reserved by caller for @buf
	*/

	memset(buf, 0, 8);

	if (value == 0) return;

	// The first byte with sign and exponent SEEEEEEE
	uint8_t left_byte = 0;
	if (value < 0)
	{
		left_byte = 0x80;
		value = -value;
	}

	const double fexp = 0.25 * log2(value);
	double exponent = ceil(fexp);
	if (exponent == fexp) exponent++;
	left_byte += (uint8_t)(64 + exponent);

	const uint64_t mantissa = (uint64_t)(value * pow(16, 14 - exponent));

	union
	{
		uint8_t a[8];
		uint64_t b;
	} tmp = {0};

	tmp.b = ((uint64_t)left_byte << 56) | (mantissa & 0x00FFFFFFFFFFFFFF);

	swap_big_endian(&tmp.b, 8);

	for (int i = 0; i < 8; i++)
	{
		buf[i] = tmp.a[i];
	}
}

static void int_to_buffer(char* p, int offset, int32_t n)
{
	/*
		Write int into buffer with offset
		Caller responsible enough is allocated for buffer
	*/

	p[offset + 0] = (n >> 24) & 0xFF;
	p[offset + 1] = (n >> 16) & 0xFF;
	p[offset + 2] = (n >> 8) & 0xFF;
	p[offset + 3] = n & 0xFF;
}

static void append_record(FILE* file, uint16_t record)
{
	/* Write a GDS record to a file */

	char buf[4];

	// The record header
	buf[0] = 0;
	buf[1] = 4;
	buf[2] = (record >> 8) & 0xFF;
	buf[3] = record & 0xFF;

	fwrite(buf, 4, 1, file);
}

static void append_short(FILE* file, uint16_t record, uint16_t data)
{
	/* Write a record with a short payload to GDS file */

	char buf[6];

	// The record header
	buf[0] = 0;
	buf[1] = 6;
	buf[2] = (record >> 8) & 0xFF;
	buf[3] = record & 0xFF;

	// The payload
	buf[4] = (data >> 8) & 0xFF;
	buf[5] = data & 0xFF;

	fwrite(buf, 6, 1, file);
}

static void append_byte(FILE* file, uint16_t record, const char* data, int count)
{
	/* Write record with byte payload to GDS file */

	char buf[4];

	// The record header
	buf[0] = ((count + 4) >> 8) & 0xFF;
	buf[1] = (count + 4) & 0xFF;
	buf[2] = (record >> 8) & 0xFF;
	buf[3] = record & 0xFF;
	fwrite(buf, 4, 1, file);

	// The payload
	fwrite(data, count, 1, file);
}

static void append_string(FILE* file, uint16_t record, const char* data)
{
	 /* Write record with string payload to GDS file */

	char buf[4];

	int countChars = (int)strlen(data);

	int record_len = countChars;

	if (countChars % 2) record_len++;

	record_len += 4; // Add 4 for the header

	// The record header
	buf[0] = (record_len >> 8) & 0xFF;
	buf[1] = record_len & 0xFF;
	buf[2] = (record >> 8) & 0xFF;
	buf[3] = record & 0xFF;

	fwrite(buf, 4, 1, file);

	// The payload
	fwrite(data, countChars, 1, file);

	// Extra 0 if countChars was odd
	if (countChars % 2)
		putc(0, file);
}

static void append_boundary(FILE* file, const gds_pair* p, int npairs, uint16_t layer)
{
	/* Add boundary element to GDS file */

	// Allocate the buffer
	char* buf = (char *) malloc(npairs * 8 * sizeof(char));

	for (int i = 0; i < npairs; i++)
	{
		assert(p[i].x <= INT32_MAX && p[i].x >= INT32_MIN);
		assert(p[i].y <= INT32_MAX && p[i].y >= INT32_MIN);

		int_to_buffer(buf, 8 * i, (int32_t)p[i].x);
		int_to_buffer(buf, 8 * i + 4, (int32_t)p[i].y);
	}

	// Write the necessary records to the file
	append_record(file, BOUNDARY);
	append_short(file, LAYER, layer);
	append_short(file, DATATYPE, 0);
	append_byte(file, XY, buf, 8 * npairs);
	append_record(file, ENDEL);

	free(buf);
}

int gds_write(const wchar_t* dest, gds_polyset* pset, double dbunit_size_uu, double dbunit_size_in_m)
{
	/* Write polygon set to a file */

	FILE* fp;
	_wfopen_s(&fp, dest, L"wb");

	if (!fp)
		return EXIT_FAILURE;

	append_short(fp, HEADER, 600);

	// Just write zeros to BGNLIB
	char zeros[24] = {0};
	append_byte(fp, BGNLIB, zeros, 24);

	// Empty library name
	append_string(fp, LIBNAME, "");

	// Write the units
	uint8_t tmp[16] = {0};
	double_to_buffer(dbunit_size_uu, tmp);
	double_to_buffer(dbunit_size_in_m, &tmp[8]);
	append_byte(fp, UNITS, (const char*)tmp, 16);

	// The cell name is "TOP"
	append_byte(fp, BGNSTR, zeros, 24);
	append_string(fp, STRNAME, "TOP");

	for (gds_polygon* poly : *pset->get())
	{
		append_boundary(fp, poly->pairs, poly->npairs, poly->layer);
	}

	// Write the tail headers
	append_record(fp, ENDSTR);
	append_record(fp, ENDLIB);

	fclose(fp);

	return EXIT_SUCCESS;
}
