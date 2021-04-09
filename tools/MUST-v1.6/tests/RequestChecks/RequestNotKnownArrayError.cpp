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
 * @file RequestNotKnownArrayError.cpp
 * This is a a test for the analysis RequestCheck.
 *
 * Warning:
 * It is difficult to create a request that is unknown but not null.
 * This test may run not on every platform as it is suggested.
 *
 * Description:
 * Performs 3 irecvs and 3 sends and do a waitall afterwards.
 * this waitall call is performed with an request array argument containing a unknown request,
 * which is not allowed.
 *
 *  @date 07.04.2011
 *  @author Mathias Korepkat
 */

#include <iostream>
#include <mpi.h>

int main (int argc, char** argv)
{
	int size,size1,size2, rank;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	MPI_Status status[4];
	MPI_Request request[4];/** 3 of 4 requests are used by Irecv, request[3] is unknown when MPI_Waitall is called **/
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
		MPI_Irecv (&size, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &(request[0]));
		MPI_Irecv (&size1, 1, MPI_INT, 0, 43, MPI_COMM_WORLD, &(request[1]));
		MPI_Irecv (&size2, 1, MPI_INT, 0, 44, MPI_COMM_WORLD, &(request[2]));
		request[3] = request[2];
		MPI_Wait(&(request[3]),&(status[3]));
		MPI_Waitall(4,request,status);
		if(request[2] != MPI_REQUEST_NULL)
			MPI_Request_free (&(request[2]));
		if(request[1] != MPI_REQUEST_NULL)
			MPI_Request_free (&(request[1]));
		if(request[0] != MPI_REQUEST_NULL)
			MPI_Request_free (&(request[0]));
	}


	if (rank == 0)
	{
		MPI_Send (&size, 1, MPI_INT, 1, 42, MPI_COMM_WORLD);
		MPI_Send (&size, 1, MPI_INT, 1, 43, MPI_COMM_WORLD);
		MPI_Send (&size, 1, MPI_INT, 1, 44, MPI_COMM_WORLD);
	}

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
