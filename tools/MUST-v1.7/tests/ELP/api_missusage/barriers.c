/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <omp.h>

int main(int argc, char **argv) {
	// Threads of a team reach two different barriers
	#pragma omp parallel
	{
  		if (omp_get_thread_num() % 2 == 0) {
			#pragma omp barrier
		} else {
			#pragma omp barrier
		}

	} // end omp parallel

	return 0;
}


