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
 * @file IntegrityIfNullAndNotMpiBottomNoError.cpp
 * This is a a test for the analysis group BasicChecks.
 *
 * Description:
* Performs a send, recv without any errors.
 *
 *  @date 27.05.2011
 *  @author Mathias Korepkat
 */

#include <iostream>
#include <mpi.h>
#include "mustTest.h"

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

	//creating a new Datatype
	int blocklens = 1;
	MPI_Aint displs;
	MPI_Address (&size, &displs);
	MPI_Datatype dtype, array_of_types = MPI_INT;

	MPI_Type_struct( 1, &blocklens, &displs, &array_of_types, &dtype );
	MPI_Type_commit(&dtype);

	//create a cartesian communicator
	if (rank == 0)
	{
		MPI_Send (&size, 1, MPI_INT, 1, 27, MPI_COMM_WORLD);
		MPI_Send (MPI_BOTTOM, 1, dtype, 1, 32, MPI_COMM_WORLD);
	}

	if (rank == 1)
	{
		MPI_Recv (&size, 1, MPI_INT, 0, 27, MPI_COMM_WORLD, &status);
		MPI_Recv (MPI_BOTTOM, 1, dtype, 0, 32, MPI_COMM_WORLD, &status);
	}


	MPI_Type_free(&dtype);

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
