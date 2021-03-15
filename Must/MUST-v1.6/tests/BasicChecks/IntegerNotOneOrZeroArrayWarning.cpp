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
 * @file IntegerNotOneOrZeroArrayWarning.cpp
 * This is a a test for the analysis group BasicChecks.
 *
 * Description:
 * Creates a cartesian communicator with an period entry of -1.
 * The entries should be 0 or 1, so this will cause a warning.
 *
 *  @date 11.04.2011
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

	//create a cartesian communicator
	MPI_Comm comm;
	int dims[3], periods[3];
	int source, dest;
	dims[0] = size;
	dims[1] = 1;
	dims[2] = 1;
	periods[0] = 1;
	periods[1] = -1; /** causes a warning **/
	periods[2] = 1;

	MPI_Cart_create( MPI_COMM_WORLD, 3, dims, periods, 1, &comm );
	MPI_Comm_free( &comm );

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
