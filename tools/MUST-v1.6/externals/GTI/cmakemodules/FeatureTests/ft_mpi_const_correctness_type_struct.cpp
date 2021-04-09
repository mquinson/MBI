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
 * This test evaluates whether we require const correctness that was introduced with MPI 3.
 * The basic const correctness test is "ft_mpi_const_correctness.c".
 *
 * This test looks at an unclear case that differs depending on MPI,
 * which is MPI_Type_struct:
 * - OpenMPI 1.8.2 (MPI-3) chooses to not use const in its definition
 * - Cray mpich 7.1.1 (MPI-?) chooses to use const in its definition
 *
 * So this specialized test case determines which handling we need for this specific function.
 * (The underlying thing is that MPI_Type_struct was deprecated, so
 *  apparently OpenMPI choose to not adapt it to const correctness,
 *  while the Cray MPICH apparently does so)
 */

int main(int argc, char **argv)
{
    /*Dummy call to avoid any optimization out, its not functional, just to please compilers*/
    const int blens[3] = {1,1,1};
    const MPI_Aint bdisps[3] = {1,1,1};
    const MPI_Datatype types[3] = {MPI_INT,MPI_INT,MPI_INT};
    MPI_Type_struct (1,blens,bdisps,types,NULL);
    return 0;
}
