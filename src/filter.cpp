/*
 * median-filter, markov-gibbs smoothing, segmentation
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "image.hpp"

void qsort_27(ushort* arr) {
	int piv, L, R, i=0;
	static ushort beg[27], end[27];

	beg[0] = 0;
	end[0] = 27;
	
	while (i >= 0) {
		L = beg[i]; R = end[i] - 1;
		if (L < R) {
			piv = arr[L];
			if (i== 26) return;
			
			while (L < R) {
				while (arr[R] >= piv && L < R) R--;
				if (L < R) arr[L++] = arr[R];
				while (arr[L] <= piv && L < R) L++;
				if (L < R) arr[R--] = arr[L];
			}
			
			arr[L] = piv;
			beg[i+1] = L + 1;
			end[i+1] = end[i];
			end[i++] = L;
		} else i--;
	}
}

/*static inline int icmp(const void* lhs, const void* rhs) {
	return (*(ushort*) lhs) > (*(ushort*) rhs);
}*/

static inline void median_voxfilter(img** volume, short z, short y, short x) {
	static ushort shades[27];
	static short idx;
	idx = 0;
	
	for (int i=z-1; i < z+2; ++i) {
		if (i < 0 || i >= num_imgs) { shades[idx] = 0; ++idx; continue; }
		for (int j=y-1; j < y+2; ++j) {
			if (j < 0 || j > 511) { shades[idx] = 0; ++idx; continue; }
			
			for (int k=x-1; k < x+2; ++k) {
				if (k < 0 || k > 511) {
					shades[idx] = 0;
				} else {
					shades[idx] = volume[i]->pix[j][k];
				}
				
				++idx;
			}
		}
	}
	
	// qsort(shades, 27, sizeof(ushort), icmp);
	qsort_27(shades);
	volume[z]->pix[y][x] = shades[13]; // median (14th - 1)
}

static inline void binary_voxfilter(img** volume, short z, short y, short x) {
	static int black;
	black = 0;
	
	for (short i=z-1; i < z+2; ++i) {
		if (i < 0 || i >= num_imgs) continue;
		
		for (short j=y-1; j < y+2; ++j) {
			if (j < 0 || j > 511) continue;
			
			for (short k=x-1; k < x+2; ++k) {
				if (k < 0 || k > 511) continue;
				
				if (volume[i]->pix[j][k] != white) {
					++black;
				}
			}
		}
	}
	
	volume[z]->pix[y][x] = (black > 13) ? 0 : white;
}

void median_volume_filter(img** volume, bool binary) {
	for (short z=0; z < num_imgs; ++z) {
		clog << (z + 1.0) / num_imgs * 100.0 << "%... ";
		
		for (short y=0; y < 512; ++y) {
			for (short x=0; x < 512; ++x) {
				if (binary) {
					binary_voxfilter(volume, z, y, x);
				} else {
					median_voxfilter(volume, z, y, x);
				}
			}
		}
	}
	
	cout << endl;
}

inline bool mgrf(img** volume, ushort k1, ushort k2, ushort k3, ushort delta, float beta) {
	short i, j, k;
	num w = 0, b = 0;

	for (i=k1-delta; i <= k1+delta; ++i) {
		if (i < 0 || i >= num_imgs) continue;
		
		for (j=k2-delta; j <= k2+delta; ++j) {
			if (j < 0 || j > 511) continue;
			
			for (k=k3-delta; k <= k3+delta; ++k) {
				if (k < 0 || k > 511) continue;
				
				if (volume[k1]->pix[k2][k3] == white) {
					++w;
				} else {
					++b;
				}
			}
		}
	}
	
	w = exp(beta * w);
	b = exp(beta * b);
	
	return w > b;
}
		
void mgrf_apply(img** volume) {	
	for (ushort n=0; n < num_imgs; ++n) {
		clog << (n + 1.0) / num_imgs * 100.0 << "%... ";
		
		for (ushort w=0; w < 512; ++w) {
			for (ushort h=0; h < 512; ++h) {
				if (mgrf(volume, n, w, h, 1, 0.3)) {
					volume[n]->pix[w][h] = white;
				}
			}
		}
	}
	
	cout << endl;
}

void segment(img** volume, ushort thresh, seg_type keep, ushort rm_val) {
	for (ushort z=0; z < num_imgs; ++z) {
		for (ushort y=0; y < 512; ++y) {
			for (ushort x=0; x < 512; ++x) {
				switch (keep) {
					case RM_ABOVE_T:
						if (volume[z]->pix[y][x] > thresh) {
							volume[z]->pix[y][x] = rm_val;
						}
						break;
					case RM_UNDER_T:
						if (volume[z]->pix[y][x] < thresh) {
							volume[z]->pix[y][x] = rm_val;
						}
						break;
					case RM_EQNOT_T:
						if (volume[z]->pix[y][x] != thresh) {
							volume[z]->pix[y][x] = rm_val;
						}
						break;
					case RM_EQUAL_T:
						if (volume[z]->pix[y][x] == thresh) {
							volume[z]->pix[y][x] = rm_val;
						}
						break;
				}
			}
		}
	}
}
