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
 * @file IntegrityIfNullStatusNoError.cpp
 * This is a a test for the analysis group BasicChecks.
 *
 * Description:
 * Performs a send, recv without any errors.
 *
 *  @date 03.06.2011
 *  @author Mathias Korepkat
 */

#include <iostream>
#include <mpi.h>

#include "mustConfig.h"

int main (int argc, char** argv)
{
	int size, rank;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	MPI_Status status;

	//Enough tasks ?
	if (size < 1)
	{
		std::cerr << "This test needs at least 1 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;


	//send
    if (rank == 0)
    {
        MPI_Send(&size, 1, MPI_INT, 1, 42, MPI_COMM_WORLD);
#ifdef HAVE_MPI_STATUS_IGNORE
        MPI_Send(&size, 1, MPI_INT, 1, 42, MPI_COMM_WORLD);
#endif /*HAVE_MPI_STATUS_IGNORE*/
    }

    //Recv
    if (rank == 1)
    {
        MPI_Recv(&size, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &status);
#ifdef HAVE_MPI_STATUS_IGNORE
        MPI_Recv(&size, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#endif /*HAVE_MPI_STATUS_IGNORE*/
    }

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
