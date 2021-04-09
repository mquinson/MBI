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
 * @file waitAnySourceEx6NoDl.c
 * Complex example that tries to enforce the "deciding" mode of must (if queues become too big).
 *
 * Description:
 * No deadlock, just a late completion.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>

int main (int argc, char** argv)
{
    int rank,size, buf, buf2, buf3, i;
    MPI_Status status;
    MPI_Request request, request2;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    //Enough tasks ?
    if (size < 4)
    {
        printf ("This test needs at least 4 processes!\n");
        MPI_Finalize();
        return 1;
    }

    //Say hello
    printf ("Hello, I am rank %d of %d processes.\n", rank, size);

    //Irecv with match, we complete it later
    MPI_Irecv (&buf3, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
    MPI_Send (&buf, 1, MPI_INT, (rank+1)%size, 666, MPI_COMM_WORLD);

    //Lots of other comm.
    for (i= 0; i < 50000; i++)
    {
        MPI_Irecv (&buf2, 1, MPI_INT, (rank-1+size)%size, 666, MPI_COMM_WORLD, &request2);
        MPI_Send (&buf, 1, MPI_INT, (rank+1)%size, 666, MPI_COMM_WORLD);
        MPI_Wait(&request2, &status);
    }

    //Now complete and do an additional round of wild-card communication
    MPI_Wait(&request, &status);
    MPI_Irecv (&buf2, 1, MPI_INT, MPI_ANY_SOURCE, 666, MPI_COMM_WORLD, &request);
    MPI_Send (&buf, 1, MPI_INT, (rank+1)%size, 666, MPI_COMM_WORLD);
    MPI_Wait(&request, &status);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    //ERROR: All ranks are lacking a Wait for the Irecv, Rank 0 lacks a Send call
    MPI_Finalize ();

    return 0;
}
