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
 * @file OnlyOnRoot_noError.cpp
 * This is a a test for the analysis group BasicChecks.
 *
 *
 * Description:
 * Performs some Gatherv calls,
 * without triggering any errors or warnings. This is just significant at root.
 *
 *  @date 25.04.2011
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

	int inBuff[2] = {1,1},
		recvCounts[2]={1,1};
	int outBuff[2] = {0,0};
	int recvDispl[2] = {0,1};

	if(rank == 1)
		MPI_Gatherv(&inBuff,1,MPI_INT,NULL,recvCounts,recvDispl,MPI_INT,0,MPI_COMM_WORLD);
	else
		MPI_Gatherv(&inBuff,1,MPI_INT,outBuff,recvCounts,recvDispl,MPI_INT,0,MPI_COMM_WORLD);

	if(rank == 1)
		MPI_Gatherv(&inBuff,1,MPI_INT,outBuff,NULL,recvDispl,MPI_INT,0,MPI_COMM_WORLD);
	else
		MPI_Gatherv(&inBuff,1,MPI_INT,outBuff,recvCounts,recvDispl,MPI_INT,0,MPI_COMM_WORLD);

	if(rank == 1)
		MPI_Gatherv(&inBuff,1,MPI_INT,outBuff,recvCounts,NULL,MPI_INT,0,MPI_COMM_WORLD);
	else
		MPI_Gatherv(&inBuff,1,MPI_INT,outBuff,recvCounts,recvDispl,MPI_INT,0,MPI_COMM_WORLD);

	if(rank == 1)
	{
		recvDispl[1] = -1;
		MPI_Gatherv(&inBuff,1,MPI_INT,outBuff,recvCounts,NULL,MPI_INT,0,MPI_COMM_WORLD);
	}else{
		MPI_Gatherv(&inBuff,1,MPI_INT,outBuff,recvCounts,recvDispl,MPI_INT,0,MPI_COMM_WORLD);
	}
	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
