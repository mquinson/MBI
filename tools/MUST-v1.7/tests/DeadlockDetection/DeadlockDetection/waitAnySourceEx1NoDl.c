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
 * @file waitAnySourceEx1NoDl.c
 * Send recv with wait and any source test with repetition (No Error).
 *
 * Description:
 * Multiple iterations of a single send-recv (with wait and wildcard receive) and a barrier
 * are executed.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size, i;
    MPI_Status status;
    MPI_Request request;

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

    	for (i = 0; i < 5; i++)
    	{
    	    if (rank == 0 || rank == 1)
    	    {
    	        if ((rank+i)%2 == 0)
    	        {
    	            MPI_Send (&size, 1, MPI_INT, (rank+1)%2, 666, MPI_COMM_WORLD);
    	        }

    	        if ((rank+i)%2 == 1)
    	        {
    	            MPI_Irecv (&size, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    	            MPI_Wait (&request, &status);
    	        }
    	    }

    	    MPI_Barrier (MPI_COMM_WORLD);
    	}

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
