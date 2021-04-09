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
 * @file reduceBcastCommNoDl.c
 * @date 09.01.2015
 *
 * A false-positive reproducer provided by Sven Buijssen. The code was translated to a C
 * version (from a fortran90 version) for convenience.
 *
 * The original description of the example was:
 * Code that illustrates a single-master-multiple-workers concept.
 * A communication group is created that spans all workers, a global
 * sum is calculated among them by means of MPI_Reduce, followed by MPI_Bcast.
 *
 * Example usage of validating this code with MUST 1.4rc1:
 * $ module load gcc/4.6.3-ubuntu openmpi/gcc4.6.x/1.6.3 must/1.4.0rc1
 * $ mpif90 mpi_deadlock_diagnosis.f90
 * $ mustrun -np 4 ./a.out       # everything is fine with "-np 2" and "-np 3"
 * $ firefox MUST_Output.html
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
  int nnumProcesses, myRankWorld;

  /* Variables to create a communication group consisting of all but master process */
  int nmax;
  int rank;
  MPI_Group myGroupAll, myGroupWorkers;

  /* MPI communicator for client communication */
  MPI_Comm myMPI_COMM_CONTINENT = MPI_COMM_NULL;

  int rankMPIOpResult = 0, crootProcessID;

  /* initialise the MPI environment */
  MPI_Init (&argc, &argv);

  /* Determine my rank in the world group and the number of processes. */
  MPI_Comm_rank (MPI_COMM_WORLD, &myRankWorld);
  MPI_Comm_size (MPI_COMM_WORLD, &nnumProcesses);

  //Enough tasks ?
  if (nnumProcesses < 4)
  {
	  printf ("This test needs at least 4 processes!\n");
	  MPI_Finalize();
	  return 1;
  }

  /* Get group associated with default communicator */
  if (MPI_Comm_group (MPI_COMM_WORLD, &myGroupAll) != MPI_SUCCESS)
  {
    MPI_Abort (MPI_COMM_WORLD, 1);
    exit (1);
  }

  /* Create a new communication group *not* spanning all processes,
   * by excluding the first one (i.e. the master process)
   */
  nmax = 1;
  rank = 0;   /* master process` rank */
  if (MPI_Group_excl (myGroupAll, nmax, &rank, &myGroupWorkers) != MPI_SUCCESS)
  {
	  MPI_Abort (MPI_COMM_WORLD, 2);
	  exit (2);
  }

  /* Create a new communicator, comprising all worker processes */
  if (MPI_Comm_create (MPI_COMM_WORLD, myGroupWorkers, &myMPI_COMM_CONTINENT) != MPI_SUCCESS)
  {
	  MPI_Abort (MPI_COMM_WORLD, 3);
	  exit (3);
  }

  /* Do a MPI_Allreduce() semi-manually
   *
   * /---------------------------------------------------------------------
   * | Here, MUST 1.4.0rc1 complains if more than 2 workers are involved
   * | that it has detected a deadlock where in fact there is none.
   * \---------------------------------------------------------------------
   */
  if (myRankWorld > 0)
  {
    nmax = 1;
    crootProcessID = 1; /**< why 1, with exactly two ranks this should be wrong.*/
    MPI_Reduce (&myRankWorld, &rankMPIOpResult, nmax, MPI_INT, MPI_SUM,
    		crootProcessID, myMPI_COMM_CONTINENT);
    MPI_Bcast (&rankMPIOpResult, nmax, MPI_INT,
    		crootProcessID, myMPI_COMM_CONTINENT);
  }

  /* Free communicator, if defined
   * (Note: Marmot 2.4.0 still complains about an existing communicator for the master
   *  process at MPI_Finalize(), which should not even exist actually. False positive.
   *  In fact, MUST 1.4.0rc1 does not complain.)
   */
  if (myMPI_COMM_CONTINENT != MPI_COMM_NULL)
  {
	  MPI_Comm_free(&myMPI_COMM_CONTINENT);
  }

  /* Free up groups */
  MPI_Group_free(&myGroupAll);
  MPI_Group_free(&myGroupWorkers);

  printf ("(%d) Hello world\n", myRankWorld);
  printf ("(%d) number of processes     : %d\n", myRankWorld, nnumProcesses);
  printf ("(%d) sum of all ranks        : %d\n", myRankWorld, rankMPIOpResult);

  MPI_Finalize();
  return 0;
}

/*EOF*/
