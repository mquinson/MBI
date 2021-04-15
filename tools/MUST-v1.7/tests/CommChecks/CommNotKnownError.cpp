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
 * @file CommNotKnownError.cpp
 * This is a test for the analysis group CommChecks.
 *
 * Description:
 * Performs a send, recv with an unknown communicator in send call.
 * This will cause an error.
 *
 *  @date 15.04.2011
 *  @author Mathias Korepkat
 */

#include <iostream>
#include <mpi.h>

int main (int argc, char** argv)
{
	int size, rank;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	MPI_Status status;

	//Enough tasks ?
	if (size < 2)
	{
		std::cerr << "This test needs at least 2 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;


	MPI_Comm comm1, comm2;
    int period = 1;
    MPI_Cart_create(MPI_COMM_WORLD, 1, &size, &period, 0, &comm1);
    comm2=comm1;
    MPI_Comm_free(&comm1);
	if (rank == 0)
	{
		MPI_Send (&size, 1, MPI_INT, 1, 42, comm2);
	}

	if (rank == 1)
	{
		MPI_Recv (&size, 1, MPI_INT, 0, 42, comm2, &status);

	}

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
