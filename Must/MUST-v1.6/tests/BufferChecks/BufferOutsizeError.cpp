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
 * @file BufferOutsizeError.cpp
 * This is a a test for the analysis BufferCheck.
 *
 * Description:
 * A buffer is attached to MPI, BufferedSend outsizing the buffer size, buffer detached
 * Should cause an outsized error
 * 
 *  @date 14.01.13
 *  @author Joachim Protze
 */

#include <iostream>
#include <mpi.h>

#define TEST_SEND_SIZE 1000000

int main (int argc, char** argv)
{
	int size,size1,size2, rank;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	//Enough tasks ?
	if (size < 2)
	{
		std::cerr << "This test needs at least 2 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

    int* buffer = new int[TEST_SEND_SIZE];

	if (rank == 1)
	{
        int buffsize;
        MPI_Pack_size(TEST_SEND_SIZE, MPI_INT, MPI_COMM_WORLD, &buffsize);
        int* mpibuff = new int[(buffsize + MPI_BSEND_OVERHEAD - 1)/sizeof(int) +1];
        MPI_Buffer_attach(mpibuff, buffsize + MPI_BSEND_OVERHEAD);
        MPI_Bsend(buffer, TEST_SEND_SIZE, MPI_INT, 0, 42, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bsend(buffer, TEST_SEND_SIZE, MPI_INT, 0, 42, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Buffer_detach(&mpibuff, &buffsize);
        delete[] mpibuff;
	}


	if (rank == 0)
	{
        MPI_Recv(buffer, TEST_SEND_SIZE, MPI_INT, 1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Recv(buffer, TEST_SEND_SIZE, MPI_INT, 1, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD);
	}
    delete[] buffer;
	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
