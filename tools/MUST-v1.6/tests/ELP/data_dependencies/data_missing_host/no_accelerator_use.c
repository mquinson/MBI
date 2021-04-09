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
	if (argc != 3) {
		fprintf(stderr, "not enough aruments!\nusage: %s (number) (number)\n");
		exit(EXIT_FAILURE);
	}

	int N = atoi(argv[1]);
	int M = atoi(argv[2]);

	int *array = (int*)malloc(N * sizeof(int));

	#pragma omp target data map(to:array[0:N-2])
	{
		#pragma omp target map(to:array[0:N])

		#pragma omp parallel for
		int i;
		for(i = 0; i < M; i++)
			array[i] = i*42;
	} // end omp target data

	return 0;
}
