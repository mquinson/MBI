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
	int i, j = 0;

	#pragma omp parallel for private(j) /*lastprivate(j)*/
	for( i = 0; i < 100; i++ )
		if( i % 42 == 0 )
			j = i;

	printf("%i\n", j);

	return 0;
}
