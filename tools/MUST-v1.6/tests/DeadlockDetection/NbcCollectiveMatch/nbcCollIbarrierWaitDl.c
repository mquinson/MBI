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
 * @file nbcCollIbarrierWaitDl.c
 * A test that deadlocks in a wait call that waits for an MPI_Ibarrier to complete (Deadlock).
 *
 * Description:
 * All processes execute an MPI_Ibarrier on the same communicator, excepting rank 0, they then wait on the resulting request and deadlock.
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
    MPI_Status status;
    MPI_Request request;
    MPI_Comm dup;

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

    //Issue an MPI_Ibarrier, rank 0 uses the dupped comm
    if (rank == 0)
    {
        MPI_Ibarrier (dup, &request);
    }
    else
    {
        MPI_Ibarrier (MPI_COMM_WORLD, &request);
    }

    //Wait and deadlock
    MPI_Wait (&request, &status);

    //Free the derived communicator
    MPI_Comm_free (&dup);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
