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
 * @file fortranc_C.cpp
 * This is a a test for the analysis group BasicChecks.
 *
 *  Description:
 *  A function that performs a MPI_Recv. This Routine is used by fortranc.f
 *
 *  @date 16.08.2011
 *  @author Mathias Korepkat
 */
#include <mpi.h>
#include "MustDefines.h"

void MyRecv (int* out)
{
	MPI_Status status;
	MPI_Recv (out, 1, MPI_INT, 0, 42, MPI_COMM_WORLD, &status);
}GENERATE_F77_BINDINGS(myrecv, MYRECV, MyRecv, (int* out), (out))
