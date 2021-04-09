/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file CommNullError.cpp
 * This is a test for the analysis group CommChecks.
 *
 * Description:
 * Performs a send, recv with MPI_COMM_NULL as communicator in send call.
 * This will cause an error.
 *
 * Positiv check is: CommNotKnownNoError
 *
 *  @date 15.04.2011
 *  @author Mathias Korepkat
 */

#include <iostream>
#include <mpi.h>
#include <omp.h>

int main (int argc, char** argv)
{
	int size, rank, provided;

	MPI_Init_thread (&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
	if (provided < MPI_THREAD_SERIALIZED){
		std::cout << "This test needs at least MPI_THREAD_SERIALIZED." << std::endl;
		MPI_Finalize ();
		return 1;
	}

	MPI_Status status;

	#pragma omp parallel num_threads(2)
	{
		if (omp_get_thread_num()==1)
		{
			MPI_Comm_size(MPI_COMM_WORLD, &size);
			MPI_Comm_rank(MPI_COMM_WORLD, &rank);
			//Say hello
			std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;
		}
	}
	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
