/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file test.cpp
 *       test driver for a simple GTI and weaver example.
 *
 *  @date 26.01.2011
 *  @author Tobias Hilbrich
 */

#include <mpi.h>
#include <iostream>

int main (int argc, char** argv)
{
	int size, rank;
	MPI_Status status;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	if (size < 2)
	{
		std::cerr << "This test needs at least two processes!" << std::endl;
		MPI_Finalize();
		return 0;
	}

	if (rank == 0)
		MPI_Send (&size, 1, MPI_INT, 1, 666, MPI_COMM_WORLD);

	if (rank == 1)
		MPI_Recv (&size, 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &status);

	MPI_Finalize ();

	return 0;
}
