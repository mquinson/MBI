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
 * @file simpleDarray.c
 * A datatype implementation test.
 * Contains no errors.
 *
 * @author Joachim Protze
 */

#include "datatypetest.h"


int main (int argc, char** argv)
{

    MPI_Init(&argc,&argv);

    MPI_Datatype        darraytype;

    int gsizes[] = {6, 6, 2};
    int distribs[] = {MPI_DISTRIBUTE_CYCLIC, MPI_DISTRIBUTE_BLOCK, MPI_DISTRIBUTE_NONE};
    int dargs[] = {1, MPI_DISTRIBUTE_DFLT_DARG, MPI_DISTRIBUTE_DFLT_DARG};
    int psizes[] = {2, 2, 1};

    MPI_Type_create_darray (4, 2, 3, gsizes, distribs, dargs, psizes, MPI_ORDER_C, MPI_INT, &darraytype);

    // call the real test - this is a macro, that directs to the specific test
    datatypeTest(darraytype);

    MPI_Type_free(&darraytype);

    MPI_Finalize ();

    return 0;
}
