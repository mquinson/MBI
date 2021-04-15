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

#pragma omp declare target
int some_func (int i) {
	return i*42;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "not enough aruments!\nusage: %s (number) (number)\n");
		exit(EXIT_FAILURE);
	}

	int N = atoi(argv[1]);
	int M = atoi(argv[2]);

	int *array = malloc(N * sizeof(int));

	//possible error if M > N
	#pragma omp target data map(to:array[0:M])
	{
		#pragma omp target map(to:array[0:M])

		#pragma omp parallel for
		int i;
		for(i = 0; i < M; i++)
			array[i] = some_func(i);

	} // end omp target data

	return 0;
}
