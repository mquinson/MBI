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

int main(int argc, char **argv) {
	size_t i;
	size_t iSize = (size_t)atoi(argv[1]) * 1024ULL * 1024ULL * 1024ULL;
	char *data;

	// Allocation on accelerator side may fail (too less memory)
	#pragma omp target map(to:iSize) map(alloc:data[0:iSize])
	{
		for (i = 0; i < iSize; i++)
			data[i] = 23;
	}

	return 0;
}
