/*
 * adaptive statistical model generation
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "common.hpp"

inline num gauss(num x, num miu, num sigma) {
	return (1.0 / (sqrt(2 * PI) * sqrt(sigma))) * exp(-(0.5 * square(x - miu)) / sigma);
}

void general_em(num* model, vector<num>& init_mu, num* x_image, ushort iters,
				num num_gauss, num** pdf) {
	num* variance = new num[(ushort) num_gauss];
	num* prob = new num[(ushort) num_gauss];
	num* P = new num[(ushort) num_gauss];
	num* A = new num[(ushort) num_gauss];
	
	// result arrays for data of fixed lengths
	num* result_ng = new num[(ushort) num_gauss];
	num result_255[255];
	
	num** dclass = new num*[(ushort) num_gauss];
	for (ushort k=0; k < num_gauss; ++k) {
		dclass[k] = new num[255];
	}
	
	for (ushort it=0; it < num_gauss; ++it) {
		variance[it] = 20;
		prob[it] = 1 / num_gauss;
		P[it] = 0;
	}
	
	for (ushort k=0; k < iters; ++k) {
		for (ushort i=0; i < 255; ++i) {
			for (ushort k2=0; k2 < num_gauss; ++k2) {
				P[k2] = gauss(i, init_mu[k2], variance[k2]);
			}
			
			mat_op(P, prob, (uint) num_gauss, mul, result_ng);
			num sum_p_prob = sum_of(result_ng, (uint) num_gauss);
			
			for (ushort k2=0; k2 < num_gauss; ++k2) {
				dclass[k2][i] = sum_p_prob ? ((prob[k2] * P[k2]) / sum_p_prob) * model[i] : 0;
			}
		}
		
		for (ushort k2=0; k2 < num_gauss; ++k2) {
			num temp;
			temp = sum_of(dclass[k2], 255);
			
			A[k2] = temp / 255;
			mat_op(dclass[k2], x_image, 255, mul, result_255);
			init_mu[k2] = sum_of(result_255, 255) / temp;
			
			mat_op(x_image, init_mu[k2], 255, sub, result_255);
			mat_op(result_255, 2, 255, pown, result_255);
			mat_op(dclass[k2], result_255, 255, mul, result_255);
			variance[k2] = sum_of(result_255, 255) / temp;
		}
		
		num sum_a = sum_of(A, (uint) num_gauss);
		
		for (ushort k2=0; k2 < num_gauss; ++k2) {
			prob[k2] = A[k2] / sum_a;
		}
	}
	
	for (ushort k2=0; k2 < num_gauss; ++k2) {
		for (ushort k=0; k < 255; ++k) {
			num aconst, bconst;
			aconst = 1 / sqrt(2 * PI * variance[k2]);
			bconst = square(k - 1 - init_mu[k2]) / (2 * variance[k2]);
			pdf[k2][k] = prob[k2] * aconst * exp(-bconst);
		}
	}
	
	del_num_table(dclass, (uint) num_gauss);
	delete[] variance;
	delete[] prob;
	delete[] P;
	delete[] A;
	delete[] result_ng;
}

ushort find_thresh(num* vhist) {
	num mu1_o = 80.0, mu2_o = 200.0;
	num prb1_o = 1.0/2.0, prb2_o = 1.0/2.0;
	num seg1_o = 10.0, seg2_o = 10.0;
	
	num dclass1[255];
	num dclass2[255];
	
	num x_image[255];
	num result[255];
	
	for (ushort it=0; it < 255; ++it) {
		x_image[it] = it;
	}

	for (ushort it=0; it < 100; ++it) {
		for (ushort i=0; i < 255; ++i) {
			static num pclass1, pclass2, psum, pc1s, pc2s;
			pclass1 = gauss(i, mu1_o, seg1_o);
			pclass2 = gauss(i, mu2_o, seg2_o);

			pc1s = prb1_o * pclass1;
			pc2s = prb2_o * pclass2;
			psum = pc1s + pc2s;

			if (psum == 0) {
				dclass1[i] = 0;
				dclass2[i] = 0;
			} else {
				dclass1[i] = vhist[i] * (pc1s / psum);
				dclass2[i] = vhist[i] * (pc2s / psum);
			}
		}

		static num sum_dc1, sum_dc2;
		sum_dc1 = sum_of(dclass1, 255); 
		sum_dc2 = sum_of(dclass2, 255);

		prb1_o = sum_dc1 / 255;
		prb2_o = sum_dc2 / 255;
		
		mat_op(dclass1, x_image, 255, mul, result);
		mu1_o = sum_of(result, 255) / sum_dc1;
		mat_op(dclass2, x_image, 255, mul, result);
		mu2_o = sum_of(result, 255) / sum_dc2;
		
		mat_op(x_image, mu1_o, 255, sub, result);
		mat_op(result, 2, 255, pown, result);
		mat_op(result, dclass1, 255, mul, result);
		seg1_o = sum_of(result, 255) / sum_dc1;
		mat_op(x_image, mu2_o, 255, sub, result);
		mat_op(result, 2, 255, pown, result);
		mat_op(result, dclass1, 255, mul, result);
		seg2_o = sum_of(result, 255) / sum_dc2;
		
		prb1_o /= (prb1_o + prb2_o);
		prb2_o /= (prb1_o + prb2_o);
	}

	prb1_o /= (prb1_o + prb2_o);
	prb2_o /= (prb1_o + prb2_o);
	
	num dgauss1[255];
	num dgauss2[255];
	
	num aconst, bconst;
	for (ushort k=0; k < 255; ++k) {
		aconst = 1 / sqrt(2 * PI * seg1_o);
		bconst = square(k - 1 - mu1_o) / (2 * seg1_o);
		dgauss1[k] = prb1_o * aconst * exp(-bconst);
		
		aconst = 1 / sqrt(2 * PI * seg2_o);
		bconst = square(k - 1 - mu2_o) / (2 * seg2_o);
		dgauss2[k] = prb2_o * aconst * exp(-bconst);
	}
	
	num destimate[255];
	mat_op(dgauss1, dgauss2, 255, add, destimate);
	mat_op(destimate, sum_of(destimate, 255), 255, divn, destimate);
	
	num error[255];
	mat_op(vhist, destimate, 255, sub, error);
	
	num sign_error[255];
	m_apply(error, sign, 255, sign_error);
	
	vector<num> positions(1, 0);
	for (short k=0; k < 254; ++k) {
		if (sign_error[k] != sign_error[k+1]) {
			positions.push_back(k);
		}
	}
	
	positions.push_back(255);
	
	vector<num> init_mu;	
	for (ushort k=0; k < positions.size() - 1; ++k) {
		if (positions[k + 1] - positions[k] > 1) {
			init_mu.push_back((positions[k + 1] + positions[k]) / 2.0);
		}
	}

	m_apply(error, abs, 255, result);
	num constant = sum_of(result, 255);
	
	num model[255];
	mat_op(result, constant, 255, divn, model);
	
	num num_gauss = init_mu.size();
	
	num** pdf = new num*[(ushort) num_gauss];
	for (ushort k=0; k < num_gauss; ++k) {
		pdf[k] = new num[255];
	}
	
	general_em(model, init_mu, x_image, 200, num_gauss, pdf);
	ar("init_mu: post gen_em", init_mu, (uint) num_gauss);
	
	ushort t1 = 90;
	while (dgauss1[t1] > dgauss2[t1]) {
		++t1;
	}
	
	// result = const .* sign(error)
	mat_op(sign_error, constant, 255, mul, result);
	
	for (ushort k=0; k < num_gauss; ++k) {
		if (init_mu[k] < (t1 + 10)) {
			mat_op(result, pdf[k], 255, mul, error);
			mat_op(dgauss1, error, 255, add, dgauss1);
		} else {
			mat_op(result, pdf[k], 255, mul, error);
			mat_op(dgauss2, error, 255, add, dgauss2);
		}
	}
	
	m_apply(dgauss1, abs, 255, dgauss1);
	m_apply(dgauss2, abs, 255, dgauss2);
	
	t1 = 90;
	while (dgauss1[t1] > dgauss2[t1]) {
		++t1;
	}

	del_num_table(pdf, (uint) num_gauss);
	
	return t1;
}
