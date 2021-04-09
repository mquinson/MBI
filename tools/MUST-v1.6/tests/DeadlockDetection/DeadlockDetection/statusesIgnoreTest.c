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
 * @file statusesIgnoreTest.c
 * Simple test with some MPI_Wait and MPI_Test calls that causes no deadlock (No Error).
 *
 * Description:
 * There is no deadlock in this test, we call correct and matching MPI calls.
 * This test makes a massive use of MPI_STATUS[ES]_IGNORE.
 *
 * @author Mathias Korepkat
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size;
    MPI_Request requests[3];
    MPI_Request requests_any[3];
    MPI_Request requests_some[3];
    int buf[9];
    int i=0;

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
    	int flag=0;
    	if (rank == 0)
    	{
    		MPI_Send (&(buf[0]), 1, MPI_INT, 1, 666, MPI_COMM_WORLD);
    		MPI_Irecv (&(buf[1]), 1, MPI_INT, MPI_ANY_SOURCE, 321, MPI_COMM_WORLD, requests);
    	    MPI_Send (&(buf[2]), 1, MPI_INT, 1, 123, MPI_COMM_WORLD);
    	    MPI_Wait (requests, MPI_STATUS_IGNORE);

    	    MPI_Irecv (&(buf[3]), 1, MPI_INT, 1, 323, MPI_COMM_WORLD, &(requests_any[0]));
    	    MPI_Irecv (&(buf[4]), 1, MPI_INT, MPI_ANY_SOURCE, 324, MPI_COMM_WORLD, &(requests_any[1]));
    	    MPI_Irecv (&(buf[5]), 1, MPI_INT, 1, 325, MPI_COMM_WORLD, &(requests_any[2]));
    	    int wany=0;
    	    MPI_Waitany (3,requests_any,&wany, MPI_STATUS_IGNORE);
    	    MPI_Waitall (3,requests_any, MPI_STATUSES_IGNORE);

    		MPI_Isend (&(buf[6]), 1, MPI_INT, 1, 423, MPI_COMM_WORLD, requests_some);
    		MPI_Isend (&(buf[7]), 1, MPI_INT, 1, 424, MPI_COMM_WORLD, &(requests_some[1]));
    		MPI_Isend (&(buf[8]), 1, MPI_INT, 1, 425, MPI_COMM_WORLD, &(requests_some[2]));
    	    int outcount = 0;
    		int out[3];
    		MPI_Waitsome (3, requests_some,&outcount,out, MPI_STATUS_IGNORE);
    		MPI_Waitall (3,requests_some, MPI_STATUSES_IGNORE);
    	}

    	if (rank == 1)
    	{
    		MPI_Irecv (&(buf[0]), 1, MPI_INT, 0, 666, MPI_COMM_WORLD, requests);
    		MPI_Isend (&(buf[1]), 1, MPI_INT, 0, 321, MPI_COMM_WORLD, &(requests[1]));
    		MPI_Irecv (&(buf[2]), 2, MPI_INT, 0, 123, MPI_COMM_WORLD, &(requests[2]));
    		MPI_Test (requests, &flag,MPI_STATUS_IGNORE);
       		MPI_Waitall (3, requests, MPI_STATUSES_IGNORE);

    		MPI_Isend (&(buf[3]), 1, MPI_INT, 0, 323, MPI_COMM_WORLD, requests_any);
    		MPI_Isend (&(buf[4]), 1, MPI_INT, 0, 324, MPI_COMM_WORLD, &(requests_any[1]));
    		MPI_Isend (&(buf[5]), 1, MPI_INT, 0, 325, MPI_COMM_WORLD, &(requests_any[2]));
    	    int index = 0;
    		MPI_Testany (3, requests_any,&index,&flag, MPI_STATUSES_IGNORE);
    	    MPI_Waitall (3,requests_any, MPI_STATUSES_IGNORE);

    		MPI_Irecv (&(buf[6]), 1, MPI_INT, 0, 423, MPI_COMM_WORLD, requests_some);
    	    MPI_Irecv (&(buf[7]), 1, MPI_INT, MPI_ANY_SOURCE, 424, MPI_COMM_WORLD, &(requests_some[1]));
    	    MPI_Irecv (&(buf[8]), 1, MPI_INT, MPI_ANY_SOURCE, 425, MPI_COMM_WORLD, &(requests_some[2]));
    	    int outcount = 0;
    		int out[3];
    		MPI_Testsome (3, requests_some,&outcount,out, MPI_STATUSES_IGNORE);
    		MPI_Waitall (3,requests_some, MPI_STATUSES_IGNORE);
    	}

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
