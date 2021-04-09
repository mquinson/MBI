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

int main(int argc, char **argv) {
	int a = 42;
	int b = 17;
	int sum = 0;

	// a and b are uninitialized on accelerator
	#pragma omp target map(alloc:a,b) map(tofrom:sum)
	{
		sum = a + b;
	}

	printf("Sum is: %i\n", sum);

	return 0;
}
