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
 * @file sendrecvNoDl.c
 * Simple sendrecv test for deadlock detection. (No Error)
 *
 * Description:
 * There is no deadlock in this test, we call a send and a matching recv.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size, sbuf, rbuf;
    MPI_Status status;

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
    		MPI_Send (&sbuf, 1, MPI_INT, 1, 666, MPI_COMM_WORLD);

    	if (rank == 1)
    		MPI_Sendrecv (&sbuf, 1, MPI_INT, 2, 123, &rbuf, 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &status);

    	if (rank == 2)
    	    MPI_Recv (&rbuf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD, &status);

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
