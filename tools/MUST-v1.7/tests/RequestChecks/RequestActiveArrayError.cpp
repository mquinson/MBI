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
 * @file RequestActiveArrayError.cpp
 * This is a a test for the analysis RequestCheck.
 *
 * Description:
 * Performs 3 MPI_Send_init/MPI_Recv_init and a MPI_Send/MPI_Recv, a MPI_Start and do a MPI_Startall() with a request array with 4 entries.
 * The entries of MPI_Send and the started request will cause an error.
 *
 *
 *  @date 08.04.2011
 *  @author Mathias Korepkat
 */

#include <iostream>
#include <mpi.h>

int main (int argc, char** argv)
{
	int size,size1,size2,size3, rank;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	MPI_Status status[4];
	MPI_Request request[4];
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
		MPI_Isend(&size, 1, MPI_INT, 1, 44, MPI_COMM_WORLD, &(request[3]));
		MPI_Start(&(request[1]));
	}

	if (rank == 1)
	{
		MPI_Recv_init (&size, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &(request[0]));
		MPI_Recv_init (&size1, 1, MPI_INT, 0, 43, MPI_COMM_WORLD, &(request[1]));
		MPI_Recv_init (&size2, 1, MPI_INT, 0, 44, MPI_COMM_WORLD, &(request[2]));
		MPI_Recv_init (&size3, 1, MPI_INT, 0, 44, MPI_COMM_WORLD, &(request[3]));

	}
	MPI_Startall(4,request);
	MPI_Waitall(4,request,status);


	if(request[3] != MPI_REQUEST_NULL)
		MPI_Request_free (&(request[3]));
	if(request[2] != MPI_REQUEST_NULL)
		MPI_Request_free (&(request[2]));
	if(request[1] != MPI_REQUEST_NULL)
		MPI_Request_free (&(request[1]));
	if(request[0] != MPI_REQUEST_NULL)
		MPI_Request_free (&(request[0]));
	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}

