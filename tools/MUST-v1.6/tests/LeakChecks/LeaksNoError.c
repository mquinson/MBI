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
 * @file LeaksNoError.c
 * This is a a test for the analysis LeakChecks.
 *
 * Description:
 * The test creates at least one resource of each resource type and frees it correctly.
 * There should not be any leaked resources or other types of errors here.
 *
 *
 *  @date 19.05.2011
 *  @author Tobias Hilbrich
 */

#include <mpi.h>

#include <stdio.h>
#include "mustTest.h"

void errFunction (MPI_Comm *comm, int *code, ...)
{
	//nothing to do
}

void opFunction (void *invec, void *inoutvec, int *len, MPI_Datatype *datatype)
{
	//No-op
}


int main (int argc, char** argv)
{
	int size, rank;

	MPI_Group group;
	MPI_Comm comm;
	MPI_Datatype type;
	MPI_Errhandler err;
	int key;
	MPI_Op op;
	MPI_Request request;

	MPI_Init (&argc, &argv);
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

	//Do the testing
	//==Group + Comm
	MPI_Comm_group (MPI_COMM_WORLD, &group);
	MPI_Comm_create (MPI_COMM_WORLD, group, &comm);

	MPI_Comm_free (&comm);
	MPI_Group_free (&group);

	//==Datatype
	MPI_Type_contiguous (2, MPI_DOUBLE, &type);
	MPI_Type_free (&type);

	//==Errorhandler
	MPI_Errhandler_create (errFunction, &err);
	MPI_Errhandler_free (&err);

	//==Keyvalue
	int extra_state=0;
	MPI_Keyval_create (MPI_NULL_COPY_FN, MPI_NULL_DELETE_FN, &key, &extra_state);
	MPI_Keyval_free (&key);

	//==Operation
	MPI_Op_create (opFunction, 1, &op);
	MPI_Op_free (&op);

	//==Request
	MPI_Send_init (&rank, 1, MPI_INT, rank, 666, MPI_COMM_WORLD, &request);
	MPI_Request_free (&request);

	//Say bye bye
	printf ("Signing off, rank %d.\n", rank);

	MPI_Finalize ();

	return 0;
}

