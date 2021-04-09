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
 * @file testCommTrack.c
 * A must test for the communicator tracking module.
 * Contains no errors.
 *
 * @author Tobias Hilbrich
 * @data 6.3.2011
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "mustFeaturetested.h"
#include "MustTypes.h"

#define TEST_COMM(C) fComm = MUST_Comm_m2i(C); MPI_Initialized ((int*)&fComm)

/**
 * Performs the following actions:
 * Creates different communicators, the expected outcomes of each test
 * are listed in the code.
 *
 * Needs exactly 5 processes.
 */
int main (int argc, char** argv)
{
	int ret;
	int rank, size;
	int fGroup;
	MustCommType fComm;
	MPI_Group  groupWorld = MPI_GROUP_NULL,
	                  partialGroup = MPI_GROUP_NULL;
	MPI_Comm commCreate = MPI_COMM_NULL,
					  commDup1 = MPI_COMM_NULL,
					  commDup2 = MPI_COMM_NULL,
					  commSplit = MPI_COMM_NULL,
					  commGraph = MPI_COMM_NULL,
					  commCart = MPI_COMM_NULL;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	printf ("Ready: %d of %d tasks.\n", rank, size);

	//Enough tasks ?
	if (size != 5)
	{
		printf ("This test needs exactly 5 processes.\n");
		MPI_Finalize ();
		exit (1);
	}

	//==1) Test predefined comms
	// -> NULL
	// -> Self
	// -> World
	TEST_COMM (MPI_COMM_NULL);
	TEST_COMM (MPI_COMM_SELF);
	TEST_COMM (MPI_COMM_WORLD);

	//==2) Test MPI_Comm_create
	//Get world group, remove ranks 1 and 3
	//Resulting group: 0->0, 1->2, 2->4
	MPI_Comm_group (MPI_COMM_WORLD, &groupWorld);
	int ranksIncl[3] = {0,2,4};
	MPI_Group_incl (groupWorld, 3, ranksIncl, &partialGroup);
	MPI_Comm_create (MPI_COMM_WORLD, partialGroup, &commCreate);
	TEST_COMM (commCreate);

	//==3) Test MPI_Comm_dup
	//Resulting group: 0->0, 1->2, 2->4
	if (commCreate != MPI_COMM_NULL)
		MPI_Comm_dup (commCreate, &commDup1);
	TEST_COMM (commDup1);

	//==4) Test MPI_Comm_split
	//Two groups: 0->2, 1->1, 2->0 | 0->4, 1->3
	int color = 0;
	if (rank >= 3)
		color = 1;
	MPI_Comm_split (MPI_COMM_WORLD, color, size - rank, &commSplit);
	TEST_COMM (commSplit);

	//==5) Test MPI_Graph_create
	//Should create some group with all processes in MPI_COMM_WORLD
	//Graph: 2->1, 2->3, 1->0, 3->4 (Tree with 2 as root)
	int indices[5] = {0,1,3,4,4};
	int edges[4] = {0, 1, 3, 4};
	MPI_Graph_create (MPI_COMM_WORLD, 5, indices, edges, 1, &commGraph);
	TEST_COMM (commGraph);

	//==6) Test MPI_Cart_create
	//Again exact group is unknown, but must not include rank 4
	int dims[2] = {2, 2};
	int periods[2] = {1, 0};
	MPI_Cart_create (MPI_COMM_WORLD, 2, dims, periods, 1, &commCart);
	TEST_COMM (commCart);

	//==7) Test MPI_Comm_dup on the graph comm
	MPI_Comm_dup (commGraph, &commDup2);
	TEST_COMM (commDup2);

	////Debug
	//if (commCart != MPI_COMM_NULL)
	//{
	//	int newRank;
	//	MPI_Comm_rank (commCart, &newRank);
	//	printf ("%d->%d\n", newRank, rank);
	//}

	//==?) Free the communicators
	if (commCreate != MPI_COMM_NULL)
		MPI_Comm_free (&commCreate);
	if (commDup1 != MPI_COMM_NULL)
		MPI_Comm_free (&commDup1);
	MPI_Comm_free (&commSplit);
	MPI_Comm_free (&commGraph);
	MPI_Comm_free (&commDup2);
	if (commCart != MPI_COMM_NULL)
		MPI_Comm_free (&commCart);

	if (groupWorld != MPI_GROUP_NULL)
		MPI_Group_free(&groupWorld);
	if (partialGroup != MPI_GROUP_NULL)
		MPI_Group_free(&partialGroup);

	printf ("Signing off: %d of %d tasks.\n", rank, size);
	MPI_Finalize ();
	return 0;
}
