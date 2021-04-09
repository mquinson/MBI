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
 * @file DLViewMulticommMultitag.c
 * Deadlock where multiple open messages could have been intended to match.
 *
 * Description:
 * Stresses the DL view due to multitude of open messages.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size, buf;
    MPI_Status status;
    MPI_Status statuses[5];
    MPI_Request requests[5];
    MPI_Comm newcomm;

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

    	MPI_Comm_dup (MPI_COMM_WORLD, &newcomm);

    	if (rank == 0)
    	{
    		MPI_Isend (&size, 1, MPI_INT, 1, 200, MPI_COMM_WORLD, &(requests[0]));
    		MPI_Isend (&size, 1, MPI_INT, 1, 201, MPI_COMM_WORLD, &(requests[1]));
    		MPI_Isend (&size, 1, MPI_INT, 1, 202, MPI_COMM_WORLD, &(requests[2]));
    		MPI_Isend (&size, 1, MPI_INT, 1, 203, MPI_COMM_WORLD, &(requests[3]));
    		MPI_Isend (&size, 1, MPI_INT, 1, 205, MPI_COMM_WORLD, &(requests[4]));

    		MPI_Recv (&buf, 1, MPI_INT, 1, 200, MPI_COMM_WORLD, &status);
    		MPI_Waitall (5, requests, statuses);
    	}

    	if (rank == 1)
    	{
    	    MPI_Isend (&size, 1, MPI_INT, 0, 100, MPI_COMM_WORLD, &(requests[0]));
    	    MPI_Isend (&size, 1, MPI_INT, 0, 101, MPI_COMM_WORLD, &(requests[1]));
    	    MPI_Isend (&size, 1, MPI_INT, 0, 1010, MPI_COMM_WORLD, &(requests[2]));
    	    MPI_Isend (&size, 1, MPI_INT, 0, 1011, MPI_COMM_WORLD, &(requests[3]));
    	    MPI_Isend (&size, 1, MPI_INT, 0, 1013, newcomm, &(requests[4]));

    	    MPI_Recv (&buf, 1, MPI_INT, 0, 100, MPI_COMM_WORLD, &status);
    	    MPI_Waitall (5, requests, statuses);
    	}

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
