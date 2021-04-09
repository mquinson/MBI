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
 * @file IntegrityNullNotMPIBottomCommSizeArrayError.cpp
 * This is a a test for the analysis group BasicChecks.
 *
 * Description:
 * Performs a MPI_Allgather with a recv buffer set to NULL a communicator > 0 and ranks defined in counts,
 * this will cause an error.
 *
 * Positiv check: IntegrityIfNullCommSizeNoError.cpp
 *
 *  @date 30.05.2011
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

    char          sendbuf[7] = {'I','T','W','o','r','k','s'},
    		      recvbuf[7] = {'I','T','W','o','r','k','s'};



	 MPI_Allgather(sendbuf,
			 1,
			 MPI_CHAR,
	         NULL,/* if the communicator is valid >0 and there are elements defined in counts,
	                  so this will cause an error */
	         1,
	         MPI_CHAR,
	         MPI_COMM_WORLD);

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
