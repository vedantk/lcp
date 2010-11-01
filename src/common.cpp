/*
 * common header impl
 * 
 * Copyright (c) 2009 Vedant Kumar <vminch@gmail.com>
 */

#include "common.hpp"

ushort num_imgs;
num NUM_EPSILON;

void hit_enter() {
	dp("\nPress 'enter' or 'return' to continue.");
	cin.get();
}

void error(const char* reason) {
	dp(reason);
	hit_enter();
	exit(2);
}

int makedir(const char* path) {
	if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
		if (errno == EEXIST) return 1;
		
		cout << path << endl;
		cout << "Could not create directory: " << path << endl;
		exit(-1);
	}
}

num sum_of(num* array, uint len) {
	num t = NUM_EPSILON;
	for (uint q=0; q < len; ++q) {
		t += array[q];
	}
	return t;
}

num square(num x) {
	return x * x;
}

num sign(num val) {
	if (val > 0) {
		return 1;
	} else if (val < 0) {
		return -1;
	} else {
		return 0;
	}
}

void mat_op(num* lhs, num* rhs, uint len, m_op op, num* result) {
	for (uint it=0; it < len; ++it) {
		switch (op) {
			case mul:
				result[it] = lhs[it] * rhs[it];
				continue;
			case divn:
				result[it] = lhs[it] / rhs[it];
				continue;
			case add:
				result[it] = lhs[it] + rhs[it];
				continue;
			case sub:
				result[it] = lhs[it] - rhs[it];
				continue;
			case pown:
				result[it] = pow(lhs[it], rhs[it]);
				continue;
		}
	}
}	

void mat_op(num* lhs, num delta, uint len, m_op op, num* result) {
	for (uint it=0; it < len; ++it) {
		switch (op) {
			case mul:
				result[it] = lhs[it] * delta;
				continue;
			case divn:
				result[it] = lhs[it] / delta;
				continue;
			case sub:
				result[it] = lhs[it] - delta;
				continue;
			case pown:
				result[it] = pow(lhs[it], delta);
				continue;
			case add:
				result[it] = lhs[it] + delta;
				continue;
		}
	}
}

void del_num_table(num** t, ushort rows) {
	for (ushort k=0; k < rows; ++k) {
		delete[] t[k];
	}
	
	delete t;
}
