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
 * @file IsIntercommError.cpp
 * This is a a test for the analysis group CommChecks.
 *
 * Description:
 * Creates a cartesian communicator
 * with an intercommunicator what will cause an error
 *
 *  @date 27.04.2011
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


	MPI_Comm comm,localcomm,intercomm;

	//create a second intracommunicator
    int color = rank % 2;
    MPI_Comm_split( MPI_COMM_WORLD, color, rank, &localcomm );

    //create a intercommunicator
    MPI_Intercomm_create( localcomm, 0, MPI_COMM_WORLD, 1-color, 42, &intercomm);


	//create a cartesian communicator

	int dims[2], periods[2];
	int source, dest;
	dims[0] = 1;
	dims[1] = 1;
	periods[0] = 1;
	periods[1] = 1;

	MPI_Cart_create( intercomm, 2, dims, periods, 0, &comm );


	if(comm != MPI_COMM_NULL)
		MPI_Comm_free( &comm );

    MPI_Comm_free(&intercomm);
    MPI_Comm_free(&localcomm);

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
