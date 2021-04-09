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
 * @file persistentDl.c
 * Simple test with an MPI_Waitall call and persistent requests, causes a deadlock (Error).
 *
 * Description:
 * The waitall waits for 4 persistent requests, of which 3 can complete and one cant,
 * the one that can't complete is associated with a wildcard receive.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size, flag;
    MPI_Status statuses[4];
    MPI_Request requests[4];
    int buf[4];

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
    	    MPI_Recv_init (&(buf[0]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &(requests[0]));
    		MPI_Send_init (&(buf[1]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &(requests[1]));
    		MPI_Recv_init (&(buf[2]), 1, MPI_INT, 2, 123, MPI_COMM_WORLD, &(requests[2]));
    		MPI_Recv_init (&(buf[3]), 1, MPI_INT, MPI_ANY_SOURCE, 444, MPI_COMM_WORLD, &(requests[3]));
    		MPI_Startall (4, requests);
    		MPI_Waitall (4, requests, statuses);
    		MPI_Request_free (&(requests[0]));
    		MPI_Request_free (&(requests[1]));
    		MPI_Request_free (&(requests[2]));
    		MPI_Request_free (&(requests[3]));
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
