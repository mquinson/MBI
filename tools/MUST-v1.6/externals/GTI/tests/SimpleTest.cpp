/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file SimpleTest.cpp
 *       test driver for a simple weaver test.
 *
 *  @date 25.08.2010
 *  @author Tobias Hilbrich
 */

#include "SimpleAPI.h"
#include <unistd.h>
#include <iostream>
#include <mpi.h>
#include <dlfcn.h>

int main (int argc, char** argv)
{
	int size, rank;
	int *array;

	/*
	 * MPI is only used to spawn multiple app processes
	 * and to use MPI communication between places.
	 */
	MPI_Init (&argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &size);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);

	array = new int [rank+1];

	for (int i = 0; i < rank + 1; i++)
	{
		array[i] = rank;
	}

	//Call the API call !!!!
	void* handle = dlopen ("libweaver-wrapp-gen-output-0.so", RTLD_LAZY | RTLD_GLOBAL);
	if (handle)
	{
		void (*testptr) (int, int*, float);
		*(void **)(&testptr) = dlsym (handle, "test1");

		for (int z = 0; z < 3; z++)
		{
			if (z != 1 || rank != 1)
			{
				if (testptr)
					(*testptr) (rank+1, array, (float)rank/3.0);
				else
					std::cout << "Error: test1 API call not present in wrapper!" << std::endl;
			}

			if (z == 1)
				usleep (1000000); //1s

			if (rank == 1 && z == 1)
			{
				if (testptr)
					(*testptr) (rank+1, array, (float)rank/3.0);
				else
					std::cout << "Error: test1 API call not present in wrapper!" << std::endl;
			}
		}

		//force shutdown
		void (*shutdown) (void);
		*(void **)(&shutdown) = dlsym (handle, "shutdown");
		if (shutdown)
			(*shutdown) ();
		else
			std::cout << "Error: failed to get shutdown function!";
	}
	else
	{
		std::cout << "Failed in dlopen!" << std::endl;
	}

	MPI_Finalize ();

	return 0;
}


extern "C" int Ptest1 (int count, int* sizes, float f)
{
	std::cout << "Ptest (count=" << count << ", sizes={";

	for (int i = 0; i < count; i++)
	{
		if (i!=0) std::cout << ", ";
		std::cout << sizes[i];
	}

	std::cout << "}, f=" << f << ")" << std::endl;

	return 123;
}

extern "C" int PnewSize (int size)
{
	std::cout << "PnewSize (size=" << size << ")" << std::endl;

	return 456;
}

extern "C" int Pshutdown (void)
{
	std::cout << "Pshutdown ()" << std::endl;

	return 789;
}
