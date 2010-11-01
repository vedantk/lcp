/*
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "image.hpp"

const char* fprefix;
const char* img_type;

void arg_help() {
	dp("lcp [images: #] [[type: 'bmp' | 'pgm']] [[fprefix: str]] {STEP: \"all\"} x/y/z floats");
}

img** do_extract_lungs(img** volume) {
	dp("smoothing images with a 3d median filter...");
	median_volume_filter(volume, false);

	dp("deleting B-regions from images...");
	static_3d_region_growing(volume, NULL, 100, stage1_lambda);

	dp("calculating lung threshold...");
	num vhist[256];
	memset(vhist, 0, 256 * sizeof(num));
	
	cum_hist(volume, vhist);	
	ushort l_thresh = find_thresh(vhist, 80, 200, 200);
	dp(l_thresh);

	dp("extracting lung region...");
	img** ex_vol;
	try {
		ex_vol = new img*[num_imgs];
		for (int i=0; i < num_imgs; ++i) {
			ex_vol[i] = new img;
		}
	} catch (bad_alloc& err) {
		img_err("extraction", "OOM");
		return NULL;
	}
	
	volume_2d_region_growing(volume, ex_vol, l_thresh, stage2_lambda);	
	
	dp("filtering extracted binary volume...");
	median_volume_filter(ex_vol, true);

	dp("removing noisy edges and regions...");
	binary_flagged_3d_region_growing(ex_vol, 0);	
	
	return ex_vol;
}

void do_vasculature(img** volume, img** ex_vol, img** orig_volume) {
	dp("extracting vasculature and potentially cancerous regions...");
	uint32_t lung_hist[256];
	memset(lung_hist, 0, 256 * sizeof(uint32_t));
	num lhsum = 0;
	
	for (int n=0; n < num_imgs; ++n) {
		for (int y=0; y < 512; ++y) {
			for (int x=0; x < 512; ++x) {
				if (ex_vol[n]->pix[y][x] != white) {
					if (orig_volume[n]->pix[y][x] != 0 ) {		
						lung_hist[ orig_volume[n]->pix[y][x] ] += 1;
						++lhsum;
					}
				}
			}
		}
	}

	num norm_lung_hist[256];
	for (int n=0; n < 256; ++n) {
		norm_lung_hist[n] = lung_hist[n] / lhsum;
	}
	
	ushort v_thresh = find_thresh(norm_lung_hist, 50, 200, 200);
	cout << "v_thresh = " << v_thresh << endl;
	
	for (int n=0; n < num_imgs; ++n) {
		for (int y=0; y < 512; ++y) {
			for (int x=0; x < 512; ++x) {
				if (ex_vol[n]->pix[y][x] == white) {
					volume[n]->pix[y][x] = white;
				} else {
					volume[n]->pix[y][x] = (orig_volume[n]->pix[y][x] > v_thresh) ? 0 : white;
				}
			}
		}
	}
	
	dp("smoothing vasculature...");
	median_volume_filter(volume, true);
	
	dp("analyzing contiguous regions...");
	lung_analysis(volume, orig_volume, v_thresh);
	
/*	dp("adding root actor...");
	for (int y=0; y < 512; ++y) {
		volume[0]->pix[y][y] = 0;
	}*/
}

void doviz(char* prefix) {
	cout << "\nPlease be patient while the 3-D software creates a visualization.\n";
	cout << "You may see a blank screen for 1 to 5 minutes, depending on how large the data is.\n";
	char cmd[1024];
	sprintf(cmd, "./viz3d %s %d %f %f %f", prefix, num_imgs, dx, dy, dz);
	system(cmd);
}

int main(int argc, char** argv) {
	NUM_EPSILON = numeric_limits<num>::epsilon();
	const char* step;

#ifdef _DEVEL	
	if (argc == 1) {
		num_imgs = 276;
		img_type = "pgm";
		fprefix = "test/1/Slice.00";
		step = "all";
		dx = dy = dz = 1.0;
	} else 
#endif
	if (argc == 8) {
		num_imgs = atoi(argv[1]);
		img_type = argv[2];
		fprefix = argv[3];
		step = argv[4];
		dx = atof(argv[5]);
		dy = atof(argv[6]);
		dz = atof(argv[7]);
	} else {
		arg_help();
		
		for (int i=0; i < argc; ++i) {
			cout << argv[i] << endl;
		}

		return EXIT_SUCCESS;
	}
	
	imw = pgm_write;
	
	cout << "(Please be patient while the data sets are loaded.)\n";
	
	img **volume, **ex_vol, **orig_volume;
	if (strcmp(step, "lungs") == 0) {
		volume = read_stack(fprefix, img_type);
		
		char nbuf[1024];
		sprintf(nbuf, "%s__volume/", fprefix);
		makedir(nbuf);
		
		ex_vol = do_extract_lungs(volume);
		write_stack(volume, nbuf);
		
		sprintf(nbuf, "%s__ex_vol/", fprefix);
		makedir(nbuf);
		write_stack(ex_vol, nbuf);

		doviz(nbuf);	
	} else if (strcmp(step, "vasc") == 0) {
		char nbuf[1024];
		sprintf(nbuf, "%s__ex_vol/", fprefix);
		ex_vol = read_stack(nbuf, "pgm");
		sprintf(nbuf, "%s__volume/", fprefix);
		volume = read_stack(nbuf, "pgm");
		orig_volume = read_stack(fprefix, img_type);
		do_vasculature(volume, ex_vol, orig_volume);
		write_stack(volume, nbuf);
		
		doviz(nbuf);
	} else {
		char nbuf[1024];
		sprintf(nbuf, "%s__final/", fprefix);
		makedir(nbuf);
		
		volume = read_stack(fprefix, img_type);
		orig_volume = read_stack(fprefix, img_type);
		ex_vol = do_extract_lungs(volume);
		do_vasculature(volume, ex_vol, orig_volume);
		write_stack(volume, nbuf);

		doviz(nbuf);
	}
	
	cout << "\nFinished.\n";
	
	return EXIT_SUCCESS;
}
