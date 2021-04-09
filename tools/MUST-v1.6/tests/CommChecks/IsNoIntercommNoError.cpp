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
 * @file IsNoIntercommNoError.cpp
 * This is a a test for the analysis group CommChecks.
 *
 * Description:
 * Performs a intercomm_merge without triggering any errors or warnings
 *
 *  @date 23.05.2011
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

	MPI_Comm localcomm, intracomm,intercomm;

	//create a second intracommunicator
    int color = rank % 2;
    MPI_Comm_split( MPI_COMM_WORLD, color, rank, &localcomm );

    //create a intercommunicator
    MPI_Intercomm_create( localcomm, 0, MPI_COMM_WORLD, 1-color, 42, &intercomm);

    //intercomm merge
    MPI_Intercomm_merge(intercomm,1,&intracomm);

	if(localcomm != MPI_COMM_NULL)
		MPI_Comm_free( &localcomm );
	if(intercomm != MPI_COMM_NULL)
		MPI_Comm_free( &intercomm );
	if(intracomm != MPI_COMM_NULL)
		MPI_Comm_free( &intracomm );
	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
