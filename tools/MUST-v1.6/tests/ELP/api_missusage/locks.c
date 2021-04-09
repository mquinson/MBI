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
	struct omp_lock_t lock;

	// Usage of uninitialized locks
	#pragma omp parallel num_threads(2)
	{
		#pragma omp sections
		{
			#pragma omp section
			{
				omp_set_lock(&lock);
				omp_unset_lock(&lock);
			}
			#pragma omp section
			{
				omp_set_lock(&lock);
				omp_unset_lock(&lock);
			}
		}
	}

	return 0;
}
