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
 * @file nbcCollIbarrierWaitallP2PDl.c
 * A test that deadlocks in an MPI_Waitall call that waits for a mix of non-blocking collectives and P2P operations (Deadlock).
 *
 * Description:
 * All processes execute an MPI_Ibarrier on the same communicator, excepting rank 0 which uses a dup on MPI_Comm world instead.
 * Additionally, processes issue some P2P operations and then execute an MPI_Waitall.
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
    MPI_Status statuses[3];
    MPI_Request requests[3];
    MPI_Comm dup;
    int buf = 777, sum = 1;

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

    requests[2] = MPI_REQUEST_NULL;
    //Do some P2P communication on processes 0 and 1
    if (rank == 0)
    {
        MPI_Irecv (&size, 1, MPI_INT, 1, 666, MPI_COMM_WORLD, &(requests[2]));
    }
    else if (rank == 1)
    {
        MPI_Isend (&size, 1, MPI_INT, 0, 666, MPI_COMM_WORLD, &(requests[2]));
    }

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
    MPI_Waitall (3, requests, statuses);

    //Free the derived communicator (though we won't get here)
    MPI_Comm_free (&dup);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
