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
 * @file collAlltoallv.c
 * A test with a correct MPI_Alltoallv call (No Error).
 *
 * Description:
 * All processes execute an MPI_Alltoallv with matching and valid arguments.
 *
 * @author Tobias Hilbrich
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
    int rank,
         size,
         *sbuf = NULL,
         *rbuf = NULL,
         *scounts = NULL,
         *rcounts = NULL,
         *sdispls = NULL,
         *rdispls = NULL,
         i,
         root;
    MPI_Status status;

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

    sbuf = (int*) malloc (sizeof(int) * size*2);
    rbuf = (int*) malloc (sizeof(int) * size*4);
    scounts = (int*) malloc (sizeof(int) * size);
    rcounts = (int*) malloc (sizeof(int) * size);
    sdispls = (int*) malloc (sizeof(int) * size);
    rdispls = (int*) malloc (sizeof(int) * size);

    for (i = 0; i < size; i++)
    {
        scounts[i] = 2; //Mathias corrected this to 1, why?
        rcounts[i] = 2;
        sdispls[i] = (size - (i+1))*2;
        rdispls[i] = i*2;
    }

    MPI_Alltoallv (sbuf, scounts, sdispls, MPI_INT, rbuf, rcounts, rdispls, MPI_INT, MPI_COMM_WORLD);

    //Say bye bye
    printf ("Signing off, rank %d.\n", rank);

    if (sbuf) free (sbuf);
    if (rbuf) free (rbuf);
    if (scounts) free (scounts);
    if (rcounts) free (rcounts);
    if (sdispls) free (sdispls);
    if (rdispls) free (rdispls);

    MPI_Finalize ();

    return 0;
}
