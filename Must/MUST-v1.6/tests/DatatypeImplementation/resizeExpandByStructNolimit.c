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
 * @file resizeExpandByStructNolimit.c
 * A datatype implementation test.
 * Contains no errors.
 *
 * @author Joachim Protze
 */

#include "datatypetest.h"


int main (int argc, char** argv)
{

    MPI_Init(&argc,&argv);

    MPI_Datatype        contitype, resizedtype;

    MPI_Type_contiguous (5, MPI_INT, &contitype);
    mpiResizedByStruct (contitype , -4, 28, &resizedtype);

    // call the real test - this is a macro, that directs to the specific test
    datatypeTest(resizedtype);

    MPI_Type_free(&resizedtype);
    MPI_Type_free(&contitype);

    MPI_Finalize ();

    return 0;
}
