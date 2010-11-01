/*
 * bmp_to_pgm.cxx
 */

#include "image.hpp"

int main(int argc, char** argv) {	
	num_imgs = atoi(argv[1]);
	img** vol = read_stack(argv[2], "bmp");
	imw = pgm_write;
	write_stack(vol, argv[2]);
	
	return 0;
}
