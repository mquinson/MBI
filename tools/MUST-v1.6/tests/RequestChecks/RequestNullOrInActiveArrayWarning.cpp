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
 * @file RequestNullOrInActiveWarning.cpp
 * This is a a test for the analysis RequestCheck.
 *
 * Description:
 * Performs 2 recv_inits and 2 sends and do a waitall before calling MPI_Start,
 * this causes a Weitall call without any request that is not null or inactive.
 *
 *  @date 07.04.2011
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

	MPI_Status status[3];
	MPI_Request request[3];


	//Enough tasks ?
	if (size < 2)
	{
		std::cerr << "This test needs at least 2 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

	if (rank == 1)
	{
		MPI_Recv_init (&size, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &(request[0]));
		MPI_Recv_init (&size, 1, MPI_INT, 0, 43, MPI_COMM_WORLD, &(request[1]));
		request[2] = MPI_REQUEST_NULL;
		MPI_Waitall(2,request,status);  /** request contains two inactive request and a request set to null. And no active request that is not null **/
		MPI_Start(&(request[0]));
		MPI_Start(&(request[1]));
		MPI_Waitall(3,request,status);

		if(request[1] != MPI_REQUEST_NULL)
			MPI_Request_free (&(request[1]));
		if(request[0] != MPI_REQUEST_NULL)
			MPI_Request_free (&(request[0]));
	}


	if (rank == 0)
	{
		MPI_Send (&size, 1, MPI_INT, 1, 42, MPI_COMM_WORLD);
		MPI_Send (&size, 1, MPI_INT, 1, 43, MPI_COMM_WORLD);
	}

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
