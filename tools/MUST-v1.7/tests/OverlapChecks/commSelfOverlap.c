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
 * @file commSelfOverlap.c
 * A must overlap test.
 * Test for selfoverlapping on repetition in communication.
 *
 * @author Joachim Protze
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "mustTest.h"



int main (int argc, char** argv)
{

    int fType, pack_size, position=0;
    char *packbuff ;
    MPI_Init(&argc,&argv);

    MPI_Datatype        vectortype, structtype;

    MPI_Type_vector (10, 1, 5, MPI_INT, &vectortype);

    int                 blocklens[3] = {1,1,1};
    MPI_Aint            displs[3] = {0,0,4};

    MPI_Type_extent(MPI_INT, displs+2);

    int *inbuf = malloc(1000000);
#ifdef HAVE_MPI_TYPE_CREATE_RESIZED
    MPI_Type_create_resized (vectortype, displs[0], displs[2], &structtype);
#else
    MPI_Datatype        types[3] = {MPI_LB, vectortype, MPI_UB};
    MPI_Type_struct (3, blocklens, displs, types, &structtype);
#endif
    MPI_Type_commit(&structtype);
    int i;
    for (i=3; i<8; i++)
    {
        MPI_Pack_size(i, structtype, MPI_COMM_SELF, &pack_size);
        packbuff = malloc(pack_size);
        position=0;
        MPI_Pack(inbuf, i, structtype, packbuff, pack_size, &position, MPI_COMM_SELF);
        position=0;
        MPI_Unpack(packbuff, pack_size, &position, inbuf, i, structtype, MPI_COMM_SELF);
        free(packbuff);
    }
    MPI_Type_free(&vectortype);
    MPI_Type_free(&structtype);
    free(inbuf);

    MPI_Finalize ();

    return 0;
}
