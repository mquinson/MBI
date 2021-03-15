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
 * @file IntegerNegativeNotProcNullArrayNoError.cpp
 * This is a test for the analysis group BasicChecks.
 *
 * Description:
 * Determing the relatve numbers of ranks in two Groups, without any error or warning.
 *
 *  @date 01.03.2011
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
	if (size < 3)
	{
		std::cerr << "This test needs at least 3 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

	if (rank == 0)
	{

		MPI_Group g1,g2;
		MPI_Comm_group( MPI_COMM_WORLD, &g1 );
		MPI_Comm_group( MPI_COMM_WORLD, &g2 );
		int ranks[3] = {0,1,MPI_PROC_NULL};
		int ranks_out[3] = {0,0,0};

        MPI_Group_translate_ranks( g1, 3, ranks, g2, ranks_out );

		MPI_Group_free( &g1 );
		MPI_Group_free( &g2 );
	}

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
