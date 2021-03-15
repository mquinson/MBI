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
 * @file sendrecvDl.c
 * Simple sendrecv deadlock. (Error)
 *
 * Description:
 * A sendrecv deadlocks here, it uses a wildcard for the receive part.
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
    		MPI_Send (&sbuf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD);

    	if (rank == 1)
    		MPI_Sendrecv (&sbuf, 1, MPI_INT, 2, 123, &rbuf, 1, MPI_INT, MPI_ANY_SOURCE, 789, MPI_COMM_WORLD, &status); /*ERROR: recv tag is wrong*/

    	if (rank == 2)
    	{
    	    MPI_Recv (&rbuf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD, &status);
    	    MPI_Send (&sbuf, 1, MPI_INT, 1, 123, MPI_COMM_WORLD);
    	}

    	//Say bye bye
    	printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
