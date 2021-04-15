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
 * @file GroupNullWarning.cpp
 * This is a test for the analysis group GroupChecks.
 *
 * Description:
 * Compares a group with MPI_GROUP_NULL, this will cause a warning.
 *
 *
 *  @date 25.05.2011
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
	if (size < 2)
	{
		std::cerr << "This test needs at least 2 processes!"<< std::endl;
		MPI_Finalize();
		return 1;
	}

	//Say hello
	std::cout << "Hello, I am rank " << rank << " of " << size << " processes." << std::endl;

	//
	//create a new group
	MPI_Group group;
	MPI_Comm_group(MPI_COMM_WORLD,&group);

	int result;
	//create a communicator
	MPI_Group_compare(MPI_GROUP_NULL, group, &result);

	//free group
	if(group != MPI_GROUP_NULL)
		MPI_Group_free(&group);


	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
