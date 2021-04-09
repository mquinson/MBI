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
 * @file nbcCollIbarrierWaitsomeDl.c
 * A test that deadlocks in an MPI_Waitsome call that waits for different non-blocking collectives (Deadlock).
 *
 * Description:
 * All processes execute an MPI_Ibarrier on the same communicator, excepting rank 0, which uses a dup of comm_world instead.
 * Processes then add an MPI_Ireduce collective with the same communicator use.
 * Finally, processes issue an MPI_Waitsome on this mix.
 * (See the not in the MPI_Waitall call to notice that we detect deadlock at an earlier execution state than the application would run into)
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
    MPI_Comm dup;
    int buf = 777, sum = 1, outcount = 0, indices[2];

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

    //Create a copy of MPI_COMM_WORLD
    MPI_Comm_dup (MPI_COMM_WORLD, &dup);

    //Issue an MPI_Ibarrier and an MPI_Ireduce, rank 0 uses the dupped comm instead of MPI_COMM_WORLD
    if (rank == 0)
    {
        MPI_Ibarrier (dup, &(requests[0]));
        MPI_Ireduce (&buf, &sum, 1, MPI_INT, MPI_SUM, 0 /*root*/, dup, &(requests[1]));
    }
    else
    {
        MPI_Ibarrier (MPI_COMM_WORLD, &(requests[0]));
        MPI_Ireduce (&buf, &sum, 1, MPI_INT, MPI_SUM, 0 /*root*/, MPI_COMM_WORLD, &(requests[1]));
    }

    //Wait and deadlock
    MPI_Waitsome (2, requests, &outcount, indices, statuses);

    //Clean up, NOTE: we can arrive here easily, since the Ireduce is handled asynchronously by non-root processes!
    MPI_Waitall (2, requests, statuses);

    //Free the derived communicator
    MPI_Comm_free (&dup);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
