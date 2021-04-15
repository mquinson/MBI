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
 * @file simpleNoLoss.c
 * A complex test case for message matching.
 *
 * Description:
 * - performs 4 waves of cyclic communication in two different communicators
 *  (MPI_COMM_WORLD, inverse of world)
 * -- 1st set of waves is in MPI_COMM_WORLD
 * -- 2nd set of waves in inverse comm
 * -- 3rd set of waves is in inverse comm with wild card
 * -- 4th set of waves is a dual wave in both MPI_COMM_WORLD and invComm where wildcards are used for MPI_COMM_WORLD
 *
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define COUNT 5

int main (int argc, char** argv)
{
    int rank, invRank, size, temp, temp2;
    MPI_Status stats[10];
    MPI_Request reqs[10];
    int i;
    int *ranksIncl;
    MPI_Group groupWorld, inverseGroup;
    MPI_Comm invComm;

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

    //==1) Create a communicator with reversed ranks
    ranksIncl = (int*) malloc (sizeof(int) * size);
    for (i = 0; i < size; i++)
    {
        ranksIncl[i] = size - i - 1;
    }
    MPI_Comm_group (MPI_COMM_WORLD, &groupWorld);
    MPI_Group_incl (groupWorld, size, ranksIncl, &inverseGroup);
    MPI_Comm_create (MPI_COMM_WORLD, inverseGroup, &invComm);

    MPI_Comm_rank (invComm, &invRank);

    //==2) Do 5 waves of cyclic communication in MPI_COMM_WORLD
    for (i = 0; i < COUNT; i++)
    {
        MPI_Isend (&size, 1, MPI_INT, (rank+1)%size, i, MPI_COMM_WORLD, reqs);
        MPI_Recv (&temp, 1, MPI_INT, ((rank-1)+size)%size, i, MPI_COMM_WORLD, stats);
        MPI_Wait (reqs, stats);
    }

    //==3) Do 5 waves of cyclic communication in invComm
    for (i = 0; i < COUNT; i++)
    {
        MPI_Isend (&size, 1, MPI_INT, (invRank+1)%size, i, invComm, reqs);
        MPI_Recv (&temp, 1, MPI_INT, ((invRank-1)+size)%size, i, invComm, stats);
        MPI_Wait (reqs, stats);
    }

    //==3) Do 5 waves of cyclic wildcard communication in invComm
    for (i = 0; i < COUNT; i++)
    {
        MPI_Isend (&size, 1, MPI_INT, (invRank+1)%size, i, invComm, reqs);
        MPI_Recv (&temp, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, invComm, stats);
        MPI_Wait (reqs, stats);
    }

    //==4) Do 5 waves of a cyclic double communication in invComm and MPI_COMM_WORLD, where wildcards are used in MPI_COMM_WORLD
    for (i = 0; i < COUNT; i++)
    {
        MPI_Isend (&size, 1, MPI_INT, (invRank+1)%size, i, invComm, reqs);
        MPI_Isend (&size, 1, MPI_INT, (rank+1)%size, i, MPI_COMM_WORLD, &(reqs[1]));
        MPI_Recv (&temp, 1, MPI_INT, ((invRank-1)+size)%size, i, invComm, stats);
        MPI_Recv (&temp2, 1, MPI_INT, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, stats);
        MPI_Waitall (2, reqs, stats);
    }

    //==5) Clean up
    MPI_Group_free (&groupWorld);
    MPI_Group_free (&inverseGroup);
    MPI_Comm_free (&invComm);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    MPI_Finalize ();

    return 0;
}
