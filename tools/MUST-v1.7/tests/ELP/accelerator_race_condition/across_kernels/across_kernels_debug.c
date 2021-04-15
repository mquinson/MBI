/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <stdio.h>

#pragma omp declare target
void sumup(int *sum) {
	int i;
	for(i = 0; i < 100; i++)
		*sum += i;
}

int main(int argc, char **argv) {
	int sum = 0;

	// Sumup is started in two kernels at the same time
	// over a sections construct inside a parallel region
	#pragma omp target data map(tofrom:sum)
	{
		printf("1: %i\n", sum);

		#pragma omp target map(tofrom:sum)
		{
			sumup( &sum );
		}

		printf("2: %i\n", sum);
	}

	printf("Sum is: %i\n", sum);

	return 0;
}
