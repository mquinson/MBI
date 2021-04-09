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

int main( int argc, char **argv )
{
	int sum = 23;

	// data is uninitialized on accelerator + not copied back afterwards
	#pragma omp target map(to:sum)
	{
		int i;
		for(i = 0; i < 100; i++)
			sum += i;
	}

	printf("Sum is: %i\n", sum);

	return 0;
}
