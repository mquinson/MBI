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
 * @file waitanyNoDlEx1.c
 * Test with multiple MPI_Waitany call causes no deadlock (No Error).
 *
 * Description:
 * There is no deadlock in this test, we call correct and matching MPI calls.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

#define NUM_REPS 100

int main (int argc, char** argv)
{
    int rank,size,i,r, index;
    MPI_Status status;
    MPI_Request requests[NUM_REPS];
    int buf[NUM_REPS];

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

    	if (rank != 0)
    	{
    	    for (i = 0; i < NUM_REPS; i++)
    	    {
    	        MPI_Send (&(buf[0]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD);
    	        MPI_Recv (&(buf[1]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &status);
    	    }
    	}

    	if (rank == 0)
    	{
    	    for (r = 1; r < size; r++)
    	    {
                for (i = 0; i < NUM_REPS; i++)
                {
                    MPI_Irecv (&(buf[i]), 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &(requests[i]));
                }

                for (i = 0; i < NUM_REPS; i++)
                {
                    MPI_Waitany (NUM_REPS, requests, &index, &status);
                    MPI_Send (&(buf[index]), 1, MPI_INT, status.MPI_SOURCE, 666, MPI_COMM_WORLD);
                }
    	    }
    	}

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
