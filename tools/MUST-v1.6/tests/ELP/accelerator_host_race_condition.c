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

int main( int argc, char **argv ) {
	int sum = 0;

	// Sumup is executed by 20 threads concurrently
	// and has a data race on 'sum'
	#pragma omp target map(tofrom:sum)
	{
		#pragma omp parallel for num_threads(20)
		int i;
		for(i = 0; i < 100; ++i)
			sum += i;
	}

	printf("Sum is: %i\n", sum);

	return 0;
}
