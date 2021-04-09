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
 * @file nbcCollIbarrierTestBgWaitP2PDl.c
 * A test that deadlocks in a wait call that waits for an MPI_Ibarrier or in a call to MPI_Recv (Deadlock).
 *
 * Description:
 * Processes execute a mix of non-blocking collectives and P2P operations.
 * The test stresses the presence of a non-blocking collective that is activated before the deadlock,
 * but that does not becomes part of the deadlock.
 * Goal is to test whether we correctly remove the WFG arcs that this active (but not waited on)
 * collective must remove.
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

    //Issue an MPI_Ibarrier, rank 0 uses the dupped comm
    if (rank == 0)
    {
        MPI_Ibarrier (MPI_COMM_WORLD, &request);
        //Wait and deadlock
        MPI_Wait (&request, &status);

    }

    if (rank == 1)
    {
        MPI_Ibarrier (MPI_COMM_WORLD, &request);
        //Deadlock now
        MPI_Recv (&size, 1, MPI_INT, 2, 666, MPI_COMM_WORLD, &status);

        //Match up, Clean up, but we won't get here
        MPI_Send (&size, 1, MPI_INT, 2, 666, MPI_COMM_WORLD);
        MPI_Wait (&request, &status);
    }

    if (rank == 2)
    {
        //Deadlock now
        MPI_Recv (&size, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &status);

        //Match barrier up, Clean up, but we won't get here
        MPI_Send (&size, 1, MPI_INT, 1, 666, MPI_COMM_WORLD);
        MPI_Ibarrier (MPI_COMM_WORLD, &request);
        MPI_Wait (&request, &status);
    }

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
