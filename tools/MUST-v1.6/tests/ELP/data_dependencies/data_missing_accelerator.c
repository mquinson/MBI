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

#pragma omp declare target
int some_func(int i) {
	return i*42;
}

int main( int argc, char **argv ) {
	int N = atoi(argv[1]);
	int i;
	int *array = (int*)malloc(N * sizeof(int));

	// possible error if N < 1024
	#pragma omp target data map(to:array[0:N])
	{
		#pragma omp target map(to:array[0:N])

		#pragma omp parallel for
		for(i = 0; i < 40960; i++)
			array[i] = some_func(array[i]);
	} // end omp target data

	return 0;
}
