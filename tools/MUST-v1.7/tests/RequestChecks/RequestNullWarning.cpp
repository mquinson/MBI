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
 * @file RequestNullWarning.cpp
 * This is a a test for the analysis RequestCheck.
 *
 * Description:
 * Performs a irecv,send with a wait.
 * The Request that is given to the wait call is set to NULL,
 * which is allowed but suspicious.
 *
 *
 *  @date 06.04.2011
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

	MPI_Status status,status1;
	MPI_Request request,request1;
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
		MPI_Irecv (&size, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &request);
		request1 = MPI_REQUEST_NULL; /* Request is set to NULL, to provoke a warning */
		MPI_Wait(&request1,&status1);
		MPI_Wait(&request,&status);
	}



	if (rank == 0)
	{
		MPI_Send (&size, 1, MPI_INT, 1, 42, MPI_COMM_WORLD);
	}

//	if(request != MPI_REQUEST_NULL)
//		MPI_Request_free (&request);
	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
