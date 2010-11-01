/*
 * calculate histograms
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "image.hpp"

void hist(img* vol, ushort z, num* hist) {
	static ushort x, y;
	
	for (y=0; y < 512; ++y) {
		for (x=0; x < 512; ++x) {
			hist[ vol[z].pix[y][x] ] += 1;
		}
	}
}

void cum_hist(img* vol, num* vhist) {
	static ushort z;
	for (z=0; z < num_imgs; ++z) {
		hist(vol, z, vhist);
	}
}
