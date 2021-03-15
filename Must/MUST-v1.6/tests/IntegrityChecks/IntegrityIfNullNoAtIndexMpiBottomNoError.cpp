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
 * @file IntegrityIfNullNoAtIndexMpiBottomNoError.cpp
 * This is a a test for the analysis group BasicChecks.
 *
 * Description:
 * Performs a Reduce_scatter call, without triggering any errors or warnings.
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
	if (size != 2)
	{
		std::cerr << "This test needs exactly 2 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;


	int inBuff[2] = {4,6},
		recvCounts[2]={0,1};
	int outBuff = 0;

	if(rank == 0)
	{
		MPI_Reduce_scatter(inBuff,NULL,recvCounts,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
	}
	else
	{
		MPI_Reduce_scatter(inBuff,&outBuff,recvCounts,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
	}
	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
