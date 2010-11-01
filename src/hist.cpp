/*
 * calculate histograms
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "image.hpp"

void hist(img** vol, ushort z, uint32_t* hist) {
	for (ushort y=0; y < 512; ++y) {
		for (ushort x=0; x < 512; ++x) {
			hist[ vol[z]->pix[y][x] ] += 1;
		}
	}
}

void cum_hist(img** vol, num* ohist) {
	uint32_t vhist[256];
	memset(vhist, 0, sizeof(uint32_t) * 256);
	
	for (ushort z=0; z < num_imgs; ++z) {
		hist(vol, z, vhist);
	}
	
	num vhsum = (512.0 * 512.0 * num_imgs) - (vhist[0] + vhist[255]);
	vhist[0] = 0;
	vhist[255] = 0;
		
	for (ushort z=0; z < 256; ++z) {
		ohist[z] = vhist[z] / vhsum;
	}
	
	cout << "vhsum: " << vhsum << endl;
}
