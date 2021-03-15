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
 * @file RankFromRangesNotInGroupNoError.cpp
 * This is a test for the analysis group BasicChecks.
 *
 * Description:
 * Creates a new group with the help of the call MPI_Group_range_excl,
 * without triggering any errors or warnings
 *
 *  @date 25.05.2011
 *  @author Mathias Korepkat
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

	MPI_Group group1, group2;
	MPI_Comm_group(MPI_COMM_WORLD,&group1);

	int n=2;
	int range[2][3] = {{0,3,3},{2,0,-17}};
	MPI_Group_range_excl(group1,n,range,&group2);



	//free groups
	if(group1 != MPI_GROUP_NULL)
	{
		MPI_Group_free(&group1);
	}

	if(group2 != MPI_GROUP_NULL)
	{
		MPI_Group_free(&group2);
	}

	//Say bye bye
	std::cout << "Signing off, rank " << rank << "." << std::endl;

	MPI_Finalize ();

	return 0;
}
