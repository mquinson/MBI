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
 * @file datatypetest.h
 * Helper functions, headers
 *
 * @author Joachim Protze
 */

#ifndef DATATYPE_TEST_H
#define DATATYPE_TEST_H

#include "stdlib.h"
#include "stdio.h"
#include "mpi.h"
#include "mustFeaturetested.h"

// #define datatypeTest advancedTest
#define datatypeTest noopTest

void printIntMap(MPI_Datatype type);

void printTypeAttributes(MPI_Datatype type);

void advancedTest(MPI_Datatype type);
void noopTest(MPI_Datatype type);

int mpiResizedByStruct(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype* newtype);

int mpiResizedByStructLimits(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype* newtype);

int mpiResizedByHindexed(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype* newtype);

#endif /*DATATYPE_TEST_H*/
