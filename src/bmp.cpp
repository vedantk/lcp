/*
 * read/write access to 512x512 bitmaps.
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "image.hpp"

static Word bfType;
static DWord bfSize = 19778;
static Word bfReserved1 = 0;
static Word bfReserved2 = 0;
static DWord bfOffBits;
static DWord biSize;
static DWord biWidth = 512;
static DWord biHeight = 512;
static Word biPlanes = 1;
static Word biBitCount = 8;
static DWord biCompression = 0;
static DWord biSizeImage;
static DWord biXPelsPerMeter = 3780;
static DWord biYPelsPerMeter = 3780;
static DWord biClrUsed = 0;
static DWord biClrImportant = 0;
static rgbaPix temp;

img* bmp_read(FILE* fptr) {
	img* dat;
	try {
		dat = new img;
	} catch (bad_alloc& err) {
		img_err("*bmp", "OOM");
		return NULL;
	}

	s_fread((char*) &bfType, sizeof(Word), 1, fptr);
	s_fread((char*) &bfSize, sizeof(DWord), 1, fptr);
	s_fread((char*) &bfReserved1, sizeof(Word), 1, fptr);
	s_fread((char*) &bfReserved2, sizeof(Word), 1, fptr);
	s_fread((char*) &bfOffBits, sizeof(DWord), 1 , fptr);
	s_fread((char*) &biSize, sizeof(DWord), 1, fptr);
	s_fread((char*) &biWidth, sizeof(DWord), 1, fptr);
	s_fread((char*) &biHeight, sizeof(DWord), 1, fptr);
	s_fread((char*) &biPlanes, sizeof(Word), 1, fptr);
	s_fread((char*) &biBitCount, sizeof(Word), 1, fptr);
	s_fread((char*) &biCompression, sizeof(DWord), 1, fptr);
	s_fread((char*) &biSizeImage, sizeof(DWord), 1, fptr);
	s_fread((char*) &biXPelsPerMeter, sizeof(DWord), 1, fptr);
	s_fread((char*) &biYPelsPerMeter, sizeof(DWord), 1, fptr);
	s_fread((char*) &biClrUsed, sizeof(DWord), 1, fptr);
	s_fread((char*) &biClrImportant, sizeof(DWord), 1, fptr);

	if (biCompression >= 1 || biWidth != 512 || biHeight != 512 || (biBitCount != 8 && biBitCount != 24)) {
		img_err("~*.bmp", "Unsupported BMP Binary.");
	}
	
	if (biBitCount < 16) {
		for (ushort n=0; n < 256; ++n) {
			s_fread((char*) &temp, 4, 1, fptr);
		}
	}

	for (int j=511; j > -1; --j) {
		if (biBitCount == 8) {
			s_fread((char*) Buffer, 1, 512, fptr);
		
			for (ushort i=0; i < 512; ++i) {
				dat->pix[j][i] = Buffer[i];
				// dat.hist[ Buffer[i] ] += 1;
			}
		} else if (biBitCount == 24) {
			s_fread((char*) Buffer24, 1, (512 * 24) / 8, fptr);
			
			for (ushort i=0; i < 512; ++i) {
				memcpy((char*) &temp, Buffer24 + 3 * i, 3);
				dat->pix[j][i] = temp.r;
				// dat.hist[ dat.pix[j][i] ] += 1;
			}
		}
	}
			
	fclose(fptr);
	return dat;
}

bool bmp_write(img* dat, FILE* fptr) {
	fwrite((char*) &bfType, 2, 1, fptr);
	fwrite((char*) &bfSize, 4, 1, fptr);
	fwrite((char*) &bfReserved1, 2, 1, fptr);
	fwrite((char*) &bfReserved2, 2, 1, fptr);
	fwrite((char*) &bfOffBits, 4, 1, fptr);
	fwrite((char*) &biSize, 4, 1, fptr);
	fwrite((char*) &biWidth, 4, 1, fptr);
	fwrite((char*) &biHeight, 4, 1, fptr);
	fwrite((char*) &biPlanes, 2, 1, fptr);
	fwrite((char*) &biBitCount, 2, 1, fptr);
	fwrite((char*) &biCompression, 4, 1, fptr);
	fwrite((char*) &biSizeImage, 4, 1, fptr);
	fwrite((char*) &biXPelsPerMeter, 4, 1, fptr);
	fwrite((char*) &biYPelsPerMeter, 4, 1, fptr);
	fwrite((char*) &biClrUsed, 4, 1, fptr);
	fwrite((char*) &biClrImportant, 4, 1, fptr);

	for (ushort n=0; n < 256; ++n) {
		temp = numToRGBA(n);
		fwrite((char*) &temp, 4, 1, fptr);
	}

	for (int j=511; j > -1; --j) {
		for (ushort i=0; i < 512; ++i) {
			Buffer[i] = dat->pix[j][i];
		}

		fwrite((char*) Buffer, 1, 512, fptr);
	}

	fclose(fptr);
	return true;	
}
