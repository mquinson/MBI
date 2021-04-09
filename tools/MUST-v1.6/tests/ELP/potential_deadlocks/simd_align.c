/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	double *a = malloc(1024 * sizeof(double));
	double *b = malloc(1024 * sizeof(double));
	double *c = malloc(1024 * sizeof(double));

	int i;
	for(i = 0; i < 1024; i++) {
		a[i] = i*2;
		b[i] = i*42;
		c[i]= 0;
	}

	// usage of aligned with unaligned data
	#pragma omp simd aligned(a,b,c:8) safelen(16)
	for(i = 0; i < 1024; i++)
		c[i] = a[i] * b[i];

	printf("%f\n", c[1]);

	return 0;
}
