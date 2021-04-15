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
 * @file RequestNullArrayError.cpp
 * This is a a test for the analysis RequestCheck.
 *
 * Description:
 * Performs 3 MPI_Send_init/MPI_Recv_init and do a MPI_Startall() with a request array with 4 entries
 * for MPI_Send on rank 0.
 * The fourth element is set to null, what will cause an error.
 *
 *
 *  @date 08.04.2011
 *  @author Mathias Korepkat
 */

#include <iostream>
#include <mpi.h>

int main (int argc, char** argv)
{
	int size, rank,recv[3];
	int i;
	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	MPI_Request request[4];
	MPI_Status status[4];
	//Enough tasks ?
	if (size < 2)
	{
		std::cerr << "This test needs at least 2 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

	if (rank == 0)
	{
		MPI_Send_init (&size, 1, MPI_INT, 1, 42, MPI_COMM_WORLD, &(request[0]));
		MPI_Send_init (&size, 1, MPI_INT, 1, 43, MPI_COMM_WORLD, &(request[1]));
		MPI_Send_init (&size, 1, MPI_INT, 1, 44, MPI_COMM_WORLD, &(request[2]));
		request[3] = MPI_REQUEST_NULL;
		i=4;
	}

	if (rank == 1)
	{
		MPI_Recv_init (recv, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &(request[0]));
		MPI_Recv_init (&(recv[1]), 1, MPI_INT, 0, 43, MPI_COMM_WORLD, &(request[1]));
		MPI_Recv_init (&(recv[2]), 1, MPI_INT, 0, 44, MPI_COMM_WORLD, &(request[2]));
		i=3;
	}
		MPI_Startall(i,request);

		MPI_Waitall(3,request,status);


	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}

