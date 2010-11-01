/*
 * applied 3d region growing
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "image.hpp"

static ushort rar_seeds[] = {0, 0, 511, 511};
static ushort car_seeds[] = {0, 511, 0, 511};

bool stage1_lambda(img** vol, img** delta, int z, int y, int x, ushort shade) {
	if (vol[z]->pix[y][x] < shade) {
		vol[z]->pix[y][x] = white;
		return true;
	} else return false;
}

bool stage2_lambda(img** vol, img** delta, int z, int y, int x, ushort shade) {
	if (vol[z]->pix[y][x] > shade && delta[z]->pix[y][x] != white) {
		delta[z]->pix[y][x] = white;
		return true;
	} else return false;
}

void volume_2d_region_growing(img** volume, img** delta, ushort shade,
	bool (*lambda)(img** vol, img** delta, int z, int y, int x, ushort shade)) {
		
	vector<ushort> rar(rar_seeds, rar_seeds + (sizeof(rar_seeds) / sizeof(ushort)));
	vector<ushort> car(car_seeds, car_seeds + (sizeof(car_seeds) / sizeof(ushort)));
	rar.reserve(512 * 256);
	car.reserve(512 * 256);
	
	for (int z=0; z < num_imgs; ++z) {
		int i, j, k1, k2;
		uint visited = 1;
		uint visiting = 0;
		
		while (visited != visiting)	{
			i = rar[visiting];
			j = car[visiting];

			for (k1 = i-1; k1 <= i+1; ++k1) {
				if (k1 > 511 || k1 < 0) continue;
				
				for (k2 = j-1; k2 <= j+1; ++k2) {
					if (k2 > 511 || k2 < 0) continue;
					
					if (lambda(volume, delta, z, k1, k2, shade)) {
						rar.push_back(k1);
						car.push_back(k2);
						visited++;
					}
				}
			}
			
			visiting++;
		}
		
		rar.clear();
		car.clear();
	}
}

void static_3d_region_growing(img** volume, img** delta, ushort shade,
	bool (*lambda)(img** vol, img** delta, int z, int y, int x, ushort shade)) {

	vector<ushort> rar(rar_seeds, rar_seeds + (sizeof(rar_seeds) / sizeof(ushort)));
	vector<ushort> car(car_seeds, car_seeds + (sizeof(car_seeds) / sizeof(ushort)));
	vector<ushort> zar(4, 0);
	rar.reserve(512 * 512 * (num_imgs / 2));
	car.reserve(512 * 512 * (num_imgs / 2));
	zar.reserve(512 * 512 * (num_imgs / 2));
							
	int i, j, z, k3, k2, k1;
	uint visited = 1;
	uint visiting = 0;
	
	while (visited != visiting)	{
		i = rar[visiting];
		j = car[visiting];
		z = zar[visiting];

		for (k3 = z-1; k3 <= z+1; ++k3) {
			if (k3 >= num_imgs || k3 < 0) continue;
			
			for (k1 = i-1; k1 <= i+1; ++k1) {
				if (k1 > 511 || k1 < 0) continue;
				
				for (k2 = j-1; k2 <= j+1; ++k2) {
					if (k2 > 511 || k2 < 0) continue;
					
					if (lambda(volume, delta, k3, k1, k2, shade)) {
						rar.push_back(k1);
						car.push_back(k2);
						zar.push_back(k3);
						visited++;
					}
				}
			}
		}

		visiting++;
	}
}

static bool locate_root_seed(img** vol, ushort match,
	vector<ushort>& rar, vector<ushort>& car, vector<ushort>& zar) {

	for (ushort _i=127; _i < 383; ++_i) {
		for (ushort _j=127; _j < 383; ++_j) {
			if (vol[0]->pix[_i][_j] == match) {
				rar.clear();
				car.clear();
				zar.clear();
				
				rar.push_back(_i);
				car.push_back(_j);
				zar.push_back(0);
				
				return true;
			}
		}
	}
	
	return false;
}

void binary_flagged_3d_region_growing(img** volume, ushort shade) {
	int i, j, z, k3, k2, k1;
	uint visited, visiting;
	vector<ushort> rar, car, zar;
	
	Byte flag = 0;
	Byte max_flag = flag;
	uint max_count = 0;
	
	while (locate_root_seed(volume, shade, rar, car, zar)) {
		visited = 1;
		visiting = 0;
		flag++;
		
		while (visited != visiting)	{
			i = rar[visiting];
			j = car[visiting];
			z = zar[visiting];

			for (k3 = z-1; k3 <= z+1; ++k3) {
				if (k3 < 0 || k3 >= num_imgs) continue;
				
				for (k1 = i-1; k1 <= i+1; ++k1) {
					if (k1 < 0 || k1 > 511) continue;
					
					for (k2 = j-1; k2 <= j+1; ++k2) {
						if (k2 < 0 || k2 > 511) continue;
						
						if (volume[k3]->pix[k1][k2] == shade) {
							rar.push_back(k1);
							car.push_back(k2);
							zar.push_back(k3);
							volume[k3]->pix[k1][k2] = flag;
							visited++;
						}
					}
				}
			}
			
			visiting++;
		}

		if (visited > max_count) {
			max_flag = flag;
			max_count = visited;
		}
	}
	
	segment(volume, max_flag, RM_EQNOT_T, white);
}
