/*
 * format-agnostic architecture for image manipulation
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */
 
#include "image.hpp"

Byte Buffer[512];
Byte Buffer24[(512 * 24) / 8];
num dx, dy, dz;

img* (*imr)(FILE*) = NULL;
bool (*imw)(img*, FILE*) = NULL;

void img_err(const char* fp, const char* reason) {
    cerr << ":: " << fp << " could not be processed.\n:: " << reason << "\n";
    hit_enter();
    exit(1);
}

rgbaPix numToRGBA(ushort n) {
    rgbaPix temp;
    temp.r = n;
    temp.g = n;
    temp.b = n;
    temp.a = 0;
    return temp;
}

void s_fread(char* arptr, size_t size, ushort elems, FILE* fptr) {
    if (fread(arptr, size, elems, fptr) != (size * elems)) {
            if (ferror(fptr) == 0) {
                    return;
            }
            
	cout << strerror(ferror(fptr)) << endl;
        perror("bad permissions/corrupted file read");
        img_err("*", "should be a valid perror above");
    }
}

img* read_img(const char* fp) {
	FILE* fptr = fopen(fp, "rb");
	if (fptr == NULL) {
		img_err(fp, "File does not exist.");
	}

	return imr(fptr);
}

bool write_img(img* dat, const char* fp) {
	FILE* fptr = fopen(fp, "wb");
	if (fptr == NULL) {
		img_err(fp, "Cannot write to file.");
	}

	return imw(dat, fptr);
}

img** read_stack(const char* inprefix, const char* img_type) {
	if (strcmp(img_type, "pgm") == 0 || strcmp(img_type, "PGM") == 0) {
		imr = pgm_read;
	} else if (strcmp(img_type, "bmp") == 0 || strcmp(img_type, "BMP") == 0) {
		imr = bmp_read;
	} else {
		error("unknown image type");
	}
	
	img** volume;
	try {
		volume = new img*[num_imgs];
	} catch (bad_alloc& err) {
		img_err(inprefix, "OOM");
		return NULL;
	}
	
	char fpath[512];
	for (ushort n=0; n < num_imgs; ++n) {
		sprintf(fpath, "%s%.3u.%s", inprefix, n+1, img_type);
		volume[n] = read_img(fpath);
	}
	
	return volume;
}

void write_stack(img** volume, const char* outprefix) {
	char fpath[512];
	for (ushort n=0; n < num_imgs; ++n) {
		sprintf(fpath, "%s%.3u.pgm", outprefix, n+1);
		if (!write_img(volume[n], fpath)) {
			error("could not write image to disk!");
		}
	}
}
