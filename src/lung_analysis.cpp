/*
 * logic for extracting and analyzing solid regions from binary lung images
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "image.hpp"

#define EDGE_DELTA	64
#define BENIGN		10 //240

#define SPHERE_DEVIATION_PERCENT	.85
#define CHEBY_DELTA 			sqrt(1 / (1 - SPHERE_DEVIATION_PERCENT))

static bool locate_3d_seed(img** vol,
	vector<ushort>& rar, vector<ushort>& car, vector<ushort>& zar) {

	for (ushort z=0; z < num_imgs; ++z) {
		for (ushort _i=EDGE_DELTA-1; _i < 511-EDGE_DELTA; ++_i) {
			for (ushort _j=EDGE_DELTA-1; _j < 511-EDGE_DELTA; ++_j) {
				if (vol[z]->pix[_i][_j] == 0) {
					rar.clear();
					car.clear();
					zar.clear();
					
					rar.push_back(_i);
					car.push_back(_j);
					zar.push_back(z);
					
					return true;
				}
			}
		}
	}
	
	return false;
}

static void process_region(img** volume, img** orig, ushort thresh,
	vector<ushort>& rar, vector<ushort>& car, vector<ushort>& zar)
{
	static num dmap[6][512][512];	
	
	// if the region is too small...
	if (rar.size() < 600) {
		cout << "--> deleted an unlikely region (size: " << rar.size() << ")\n";
		return;
	}
	
	// if the region spans too many slices vertically...
	int region_zmin = *min_element(zar.begin(), zar.end());
	if (*max_element(zar.begin(), zar.end()) - region_zmin > 5) {
		dp("--> deleted an unlikely region (z-interval > 5)");
		return;
	}
	
	// find the boundary of the region
	vector<ushort> edge_rar, edge_car, edge_zar;
	
	for (ushort i=0; i < zar.size(); ++i) {
		for (ushort q=zar[i]-1; q < zar[i]+2; ++q) {
			for (ushort k=rar[i]-1; k < rar[i]+2; ++k) {
				for (ushort j=car[i]-1; j < car[i]+2; ++j) {
					if (volume[q]->pix[k][j] == white) {
						edge_rar.push_back(rar[i]);
						edge_car.push_back(car[i]);
						edge_zar.push_back(zar[i]);
						volume[zar[i]]->pix[rar[i]][car[i]] = 0;
						break;
					}
				}
				
				if (volume[zar[i]]->pix[rar[i]][car[i]] == 0) {
					break;
				}
			}
			
			if (volume[zar[i]]->pix[rar[i]][car[i]] == 0) {
				break;
			}
		}
	}
	
	ar("--> region-boundary z-vec", edge_zar, edge_zar.size()); 
	
	// create a distance map
	uint blen = edge_rar.size();
	
	num *erar, *ecar, *ezar, *dvec, *tmp;
	try {
		erar = new num[blen];
		ecar = new num[blen];
		ezar = new num[blen];
		dvec = new num[blen];
		tmp = new num[blen];
	} catch (bad_alloc& err) {
		dp("Fatal memory allocation error; results may be unreliable!");
		return;
	}
	
	for (uint i=0; i < blen; ++i) {
		// copy the boundary voxels into num-arrays to make things simple
		erar[i] = (num) edge_rar[i];
		ecar[i] = (num) edge_car[i];
		ezar[i] = (num) edge_zar[i];
	}
	
	// find the minimum distance from the boundary for each point
	for (uint i=0; i < zar.size(); ++i) {
		mat_op(erar, (num) rar[i], blen, sub, tmp);
		mat_op(tmp, dx, blen, mul, tmp);
		m_apply(tmp, square, blen, dvec);

		mat_op(ecar, (num) car[i], blen, sub, tmp);
		mat_op(tmp, dy, blen, mul, tmp);
		m_apply(tmp, square, blen, tmp);
		mat_op(tmp, dvec, blen, add, dvec);
		
		mat_op(ezar, (num) zar[i], blen, sub, tmp);
		mat_op(tmp, dz, blen, mul, tmp);
		m_apply(tmp, square, blen, tmp);
		mat_op(tmp, dvec, blen, add, dvec);
		
		m_apply(dvec, sqrt, blen, dvec);
		
		// store the distance into the distance map
		dmap[zar[i] - region_zmin][rar[i]][car[i]] = *min_element(dvec, dvec+blen);
	}
	
	// find the 'centroid' of the region
	num cen_max = 0;
	ushort cenz=0, cenx=0, ceny=0;
	
	for (ushort i=0; i < zar.size(); ++i) {
		num cur_val = dmap[zar[i] - region_zmin][rar[i]][car[i]];
		if (cur_val > cen_max) {
			cen_max = cur_val;
			cenz = zar[i] - region_zmin;
			cenx = rar[i];
			ceny = car[i];
		}
	}
	
	cout << "--> region centroid: [" << cenx << ", " << ceny << ", " << region_zmin + cenz << "]\n"; 
	
	// find the variance of the distance from the boundary to the centroid
	for (ushort i=0; i < blen; ++i) {
		tmp[i] = sqrt(square((erar[i] - cenx) * dx)
					+ square((ecar[i] - ceny) * dy)
					+ square((ezar[i] - cenz) * dz));
	}
	
	num edge_u = sum_of(tmp, blen) / blen;
	mat_op(tmp, edge_u, blen, sub, tmp);
	m_apply(tmp, square, blen, tmp);
	num stddev = sqrt(sum_of(tmp, blen) / blen);
	cout << "--> (stddev, mean) distance from centroid: (" << stddev << ", " << edge_u << ")\n";
	
	/* My thinking is that 85% or more of the distances should lie within 3 to
	 * 7 units of the mean distance to have a sphere (roughly). According to
	 * Chebyshev's rule [p = 1 - k^-2 => k = sqrt(1 / (1 - p))], 85% of the
	 * distances should lie within 2.582 standard deviations of the mean
	 * distance. So to be spherical, 3 < u - [u - (2.582 * stddev)] < 7.
	 */

	num dist_delta =  edge_u - (edge_u - (CHEBY_DELTA * stddev));
	bool tumor = (dist_delta > 3) && (dist_delta < 7);
	cout << "--> dist_delta: " << dist_delta << endl;
	cout << "--> size: " << rar.size() << endl;
	
	if (tumor) {
		uint64_t sum_shade = 0;
		for (uint i=0; i < zar.size(); ++i) {
			sum_shade += orig[zar[i]]->pix[rar[i]][car[i]];
		}
	
		tumor = (sum_shade / zar.size()) > thresh; // 67% empirically
	}
	
	for (uint i=0; i < zar.size(); ++i) {
		volume[zar[i]]->pix[rar[i]][car[i]] = tumor ? 1 : BENIGN;
	}
	
	cout << "--> " << tumor  << " tumor.\n";
	
	delete[] dvec;
	delete[] tmp;
	delete[] erar;
	delete[] ecar;
	delete[] ezar;
}

void lung_analysis(img** volume, img** orig, ushort thresh) {
	int i, j, z, k3, k2, k1;
	uint visited, visiting;
	vector<ushort> rar, car, zar;		

	while (locate_3d_seed(volume, rar, car, zar)) {
		visited = 1;
		visiting = 0;
		
		while (visited != visiting)	{
			i = rar[visiting];
			j = car[visiting];
			z = zar[visiting];

			for (k3 = z-1; k3 <= z+1; ++k3) {
				if (k3 < 0 || k3 >= num_imgs) continue;
				
				for (k1 = i-1; k1 <= i+1; ++k1) {
					if (k1 < EDGE_DELTA-1 || k1 > 511-EDGE_DELTA) continue;
					
					for (k2 = j-1; k2 <= j+1; ++k2) {
						if (k2 < EDGE_DELTA-1 || k2 > 511-EDGE_DELTA) continue;
						
						if (volume[k3]->pix[k1][k2] == 0) {
							rar.push_back(k1);
							car.push_back(k2);
							zar.push_back(k3);
							volume[k3]->pix[k1][k2] = BENIGN; // visited and benign
							visited++;
						}
					}
				}
			}
			
			visiting++;
		}
		
		process_region(volume, orig, thresh, rar, car, zar);
	}
}
