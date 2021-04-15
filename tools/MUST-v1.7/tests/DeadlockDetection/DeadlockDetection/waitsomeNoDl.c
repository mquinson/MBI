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
 * @file waitsomeNoDl.c
 * Simple test with an MPI_Waitsome call causes no deadlock (No Error).
 *
 * Description:
 * There is no deadlock in this test, we call correct and matching MPI calls.
 * We execute 1 to 3 calls to MPI_Waitsome depending on the interleaving.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size,i, outcount, indices[3], num;
    MPI_Status statuses[3];
    MPI_Request requests[3];
    int buf[3];

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    //Enough tasks ?
    	if (size < 3)
    	{
    		printf ("This test needs at least 3 processes!\n");
    		MPI_Finalize();
    		return 1;
    	}

    	//Say hello
    	printf ("Hello, I am rank %d of %d processes.\n", rank, size);

    	if (rank == 0)
    	{
    		MPI_Send (&(buf[0]), 1, MPI_INT, 1, 666, MPI_COMM_WORLD);
    		MPI_Recv (&(buf[1]), 1, MPI_INT, 1, 666, MPI_COMM_WORLD, statuses);
    	}

    	if (rank == 1)
    	{
    		MPI_Irecv (&(buf[0]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &(requests[0]));
    		MPI_Isend (&(buf[1]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &(requests[1]));
    		MPI_Irecv (&(buf[2]), 2, MPI_INT, 2, 123, MPI_COMM_WORLD, &(requests[2]));

    		i=0;
    		num =0;
    		while (i < 3)
    		{
    		    MPI_Waitsome (3, requests, &outcount, indices, statuses);
    		    i += outcount;
    		    num++;
    		}
    		printf ("Finished up with %d calls to MPI_Waitsome.\n", num);
    	}

    	if (rank == 2)
    	{
    	    MPI_Send (&(buf[0]), 1, MPI_INT, 1, 123, MPI_COMM_WORLD);
    	}

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
