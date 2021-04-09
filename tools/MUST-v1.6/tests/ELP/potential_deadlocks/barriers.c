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
#include <time.h>

int main(int argc, char **argv) {
	// Not all threads may reach the omp barrier
	#pragma omp parallel num_threads(4)
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		if(tv.tv_usec % 2 == 0) {
			#pragma omp barrier
		}
	}
	return 0;
}
