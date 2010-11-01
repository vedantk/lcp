/*
 * read/write access to 512x512 PGM files.
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "image.hpp"

static int PNMReaderGetChar(FILE* fp) {
	static char c;
	static int result;

	if ((result = getc(fp)) == EOF)	{
		return '\0';
	}

	c = (char) result;
	if (c == '#') {
		do {
			if ((result = getc(fp)) == EOF) {
				return '\0';
			}
			c = (char) result;
		} while (c != '\n');
	}

	return c;
}

static int PNMReaderGetInt(FILE* fp) {
	char c;
	int result = 0;

	do {
		c = PNMReaderGetChar(fp);
	} while ((c < '1') || (c > '9'));

	do {
		result = result * 10 + (c - '0');
		c = PNMReaderGetChar(fp);
	} while ((c >= '0') && (c <= '9'));

	ungetc(c, fp);
	return result;
}

img pgm_read(FILE* fptr) {
	img dat;
	static char magic[3];
	static int i;
	static char c;

	do {
		c = PNMReaderGetChar(fptr);
	} while (c != 'P');

	magic[0] = c;
	magic[1] = PNMReaderGetChar(fptr);
	magic[2] = '\0';

	i = PNMReaderGetInt(fptr); // rows
	i = PNMReaderGetInt(fptr); // cols
	i = PNMReaderGetInt(fptr); // max grayscale value
	
	c = getc(fptr);
	if (c == 0x0d) {
		c = getc(fptr);
		if (c != 0x0a) {
			ungetc(c, fptr);
		}
	}

	if (strncmp(magic, "P5", 2)) {
		img_err("~*.pgm", "Unsupported PGM Binary.");
	}

	for (int j=0; j < 512; ++j) {
		s_fread((char*) Buffer, 1, 512, fptr);
		
		for (ushort i=0; i < 512; ++i) {
			dat.pix[j][i] = Buffer[i];
			// dat.hist[ Buffer[i] ] += 1;
		}
	}
	
	fclose(fptr);
	return dat;
}

bool pgm_write(img* dat, FILE* fptr) {
	fprintf(fptr, "P5\n512 512\n255\n");
	
	for (int j=0; j < 512; ++j) {
		for (ushort i=0; i < 512; ++i) {
			Buffer[i] = dat->pix[j][i];
		}

		fwrite((char*) Buffer, 1, 512, fptr);
	}
	
	fclose(fptr);
	return true;
}
