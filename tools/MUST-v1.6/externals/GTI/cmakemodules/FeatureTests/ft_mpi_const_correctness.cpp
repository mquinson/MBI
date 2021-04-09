/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <mpi.h>
#include <stddef.h>

/**
 * This test evaluates whether we require const correctness that was introduced with MPI 3(?).
 * MPI_Graph_create is available in all MPI versions, thus:
 * - if this test succeeds, we have const correctness
 * - if this test fails, we either lack mpi.h or we don't require const correctness (we hopefully catch the first issue earlier)
 */

int main(int argc, char **argv)
{
  /*Dummy call to avoid any optimization out, its not functional, just to please compilers*/
  const int i=0;
  MPI_Graph_create (MPI_COMM_WORLD,0, &i, &i, 0, NULL);
  return 0;
}
