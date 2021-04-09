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

int main(int argc, char **argv)
{
  MPI_Errhandler errh=MPI_ERRHANDLER_NULL;
  MPI_Comm_set_errhandler(MPI_COMM_SELF, errh);
  PMPI_Comm_set_errhandler(MPI_COMM_SELF, errh);
  return 0;
}
