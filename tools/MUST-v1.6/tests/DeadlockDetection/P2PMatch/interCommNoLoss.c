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
 * @file simpleNoLoss.c
 * Simple send recv test for lost message detector.
 *
 * Description:
 * There is no lost message in this test.
 * We call multiple isends and irecvs while using an intercomm for comunicating.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank, oldRank,size, newSize,recv[5];
    MPI_Status stats[10];
    MPI_Request reqs[10];
    int i;
    MPI_Comm commOddEven, commInter;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    oldRank = rank;
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    //Enough tasks ?
    	if (size < 2)
    	{
    		printf ("This test needs at least 2 processes!\n");
    		MPI_Finalize();
    		return 1;
    	}

    	//Say hello
    	printf ("Hello, I am rank %d of %d processes.\n", rank, size);
    	MPI_Comm_split (MPI_COMM_WORLD, rank%2, size - rank, &commOddEven);
    	MPI_Comm_size (commOddEven, &newSize);
    	MPI_Intercomm_create (commOddEven, newSize-1, MPI_COMM_WORLD, (rank+1)%2, 666, &commInter);

    	MPI_Comm_rank (commInter, &rank);

    	if (rank == 0 && oldRank % 2)
    	{
    		for (i = 0; i < 5; i++)
    		{
    			MPI_Isend (&size, 1, MPI_INT, 0, i, commInter, &(reqs[i]));
    		}

    		for (i = 5; i < 10; i++)
    		{
    			MPI_Irecv (&(recv[i-5]), 1, MPI_INT, 0, i, commInter, &(reqs[i]));
    		}
    	}

    	if (rank == 0 && oldRank %2 == 0)
    	{
    		for (i = 0; i < 5; i++)
    		{
    			MPI_Irecv (&(recv[i]), 1, MPI_INT, 0, 4-i, commInter, &(reqs[i]));
    		}

    		for (i = 5; i < 10; i++)
    		{
    			MPI_Isend (&size, 1, MPI_INT, 0, 14-i, commInter, &(reqs[i]));
    		}
    	}

    	if (rank == 0)
    		MPI_Waitall (10, reqs, stats);

    MPI_Comm_free (&commInter);
    	MPI_Comm_free (&commOddEven);

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
