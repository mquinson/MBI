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
 * @file wcIrecvExNoLoss
 * Multiple wildcard receives from two tasks.
 *
 * Description:
 * There is no lost message in this test.
 * We call multiple isends and irecvs.
 * We use wildcard Irecvs
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size,recv[5];
    MPI_Status stats[10];
    MPI_Request reqs[10];
    int i;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
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

    	if (rank == 0)
    	{
    		for (i = 0; i < 5; i++)
    		{
    			MPI_Isend (&size, 1, MPI_INT, 1, i, MPI_COMM_WORLD, &(reqs[i]));
    		}

    		for (i = 5; i < 10; i++)
    		{
    			MPI_Irecv (&(recv[i-5]), 1, MPI_INT, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &(reqs[i]));
    		}
    	}

    	if (rank == 1)
    	{
    		for (i = 0; i < 5; i++)
    		{
    			MPI_Irecv (&(recv[i]), 1, MPI_INT, MPI_ANY_SOURCE, 4-i, MPI_COMM_WORLD, &(reqs[i]));
    		}

    		for (i = 5; i < 10; i++)
    		{
    			MPI_Isend (&size, 1, MPI_INT, 0, 14-i, MPI_COMM_WORLD, &(reqs[i]));
    		}
    	}

    	if (rank == 0 || rank == 1)
    	    MPI_Waitall (10, reqs, stats);

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
