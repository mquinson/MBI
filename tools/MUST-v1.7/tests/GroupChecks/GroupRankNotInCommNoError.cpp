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
 * @file GroupRankNotInCommNoError.cpp
 * This is a test for the analysis group GroupChecks.
 *
 * Description:
 * Creates a communicator with the help of Comm_create call, without triggering any errors or warnings
 *
 *
 *  @date 23.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include <iostream>
#include <mpi.h>

int main (int argc, char** argv)
{
	int size, rank;

	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	MPI_Status status;

	//Enough tasks ?
	if (size < 4)
	{
		std::cerr << "This test needs at least 4 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

	//
	//create a new group
	MPI_Comm newcomm,oldcomm;
	MPI_Group group;
	MPI_Comm_group(MPI_COMM_WORLD,&group);

	//create a communicator to build a new communicator with
	if(rank == 3)
	{
		MPI_Comm_split(MPI_COMM_WORLD,1,rank,&oldcomm);
	}
	else
	{
		MPI_Comm_split(MPI_COMM_WORLD,1,rank,&oldcomm);
	}

	//create a communicator
	MPI_Comm_create(oldcomm, group, &newcomm);

	//free group and communicator
	if(newcomm != MPI_COMM_NULL)
		MPI_Comm_free(&newcomm);

	if(oldcomm != MPI_COMM_NULL)
		MPI_Comm_free(&oldcomm);

	if(group != MPI_GROUP_NULL)
		MPI_Group_free(&group);

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
