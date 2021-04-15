/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <stddef.h>

#pragma omp declare target
int some_func (int i) {
	return i*42;
}

int main (int argc, char **argv) {
	int *array = NULL;

	// error: copy of NULL pointer, segfault
	#pragma omp target data map(to:array[0:1024])
	{
		#pragma omp target map(to:array[0:1024])

		#pragma omp parallel for
		int i;
		for(i = 0; i < 1024; i++)
			array[i] = some_func(i);

	} // end omp target data

	return 0;
}
