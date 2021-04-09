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
 * @file nbcCollIreduceBufferOverlapError.c
 * A test with MPI_Ireduce calls that use overlapping buffers (Error).
 *
 * Description:
 * Each process executes two MPI_Ireduce calls and then waits for any of them to complete.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
    int rank,
         size;
    MPI_Status statuses[2];
    MPI_Request requests[2];
    int i, index;
    int buf = 7, sum;

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

    MPI_Ireduce (&buf, &sum, 1, MPI_INT, MPI_SUM, 0 /*root*/, MPI_COMM_WORLD, &(requests[0]));
    MPI_Ireduce (&buf, &sum, 1, MPI_INT, MPI_SUM, 0 /*root*/, MPI_COMM_WORLD, &(requests[1]));

    MPI_Waitall (2, requests, statuses);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
