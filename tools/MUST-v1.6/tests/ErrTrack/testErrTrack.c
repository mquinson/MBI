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
 * @file testErrTrack.c
 * A must test case for errorhandler tracking.
 *
 * @author Tobias Hilbrich
 *
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mustFeaturetested.h"
#include "mustTest.h"

#define TEST_ERR(R) fErr = MUST_Errhandler_m2i (R); MPI_Initialized ((int*)&fErr)

void errFunction (MPI_Comm *comm, int *code, ...)
{
	//nothing to do
}

/**
 * Performs the following actions:
 * (Using any number of processes)
 *
 * 1) Tests predefined errhandlers
 * 2) Creates a new errhandler
 * 3) Frees the errhandler
 */
int main (int argc, char** argv)
{
	int ret;
	int rank,size;

	MPI_Errhandler err;
	MustErrType fErr;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	printf ("Ready: %d of %d tasks.\n", rank, size);

	//Enough tasks ?
	if (size < 1)
	{
		printf ("Not enough tasks, need 1 at least.\n");
		MPI_Finalize ();
		exit (1);
	}

	//==1) Base Errhandlers
	TEST_ERR (MPI_ERRORS_ARE_FATAL);
    TEST_ERR (MPI_ERRORS_RETURN);

	//==2) Errhandler create
	MPI_Errhandler_create (errFunction, &err);
	TEST_ERR (err);

	//==3) Errhandler free
	MPI_Errhandler_free (&err);
    assert(err == MPI_ERRHANDLER_NULL);
	TEST_ERR (err);

	printf ("Signing off: %d of %d tasks.\n", rank, size);
	MPI_Finalize ();
	return 0;
}
