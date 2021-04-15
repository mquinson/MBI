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
 * @file wcIrecvEx2NoLoss.c
 * Test with wildcards and MPI_ANY_TAG.
 *
 * Description:
 * There is no lost message in this test.
 * 1-3 send 3 messages to 0, which receives with wildacrd and any tag.
 * All non-blocking.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size,recv[9];
    MPI_Status stats[10];
    MPI_Request reqs[10];
    int i;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    //Enough tasks ?
    	if (size < 4)
    	{
    		printf ("This test needs at least 4 processes!\n");
    		MPI_Finalize();
    		return 1;
    	}

    	//Say hello
    	printf ("Hello, I am rank %d of %d processes.\n", rank, size);

    	if (rank == 0)
    	{
    		for (i = 0; i < 9; i++)
    		{
    			MPI_Irecv (&(recv[i]), 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &(reqs[i]));
    		}
    		i=9;
    	}

    	if (rank >= 1 && rank <=3)
    	{
    		for (i = 0; i < 3; i++)
    		{
    			MPI_Isend (&size, 1, MPI_INT, 0, i, MPI_COMM_WORLD, &(reqs[i]));
    		}
    		i = 3;
    	}

    	MPI_Waitall (i, reqs, stats);

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
