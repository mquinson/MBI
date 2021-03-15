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
 * @file datatypetest.c
 * Helper functions.
 *
 * @author Joachim Protze
 */

#include "datatypetest.h"

void printTypeAttributes(MPI_Datatype type)
{
#ifdef HAVE_MPI_TYPE_GET_TRUE_EXTENT
    int size;
    MPI_Aint true_lb, true_extent, lb, extent, ub;

    MPI_Type_extent(type, &extent);
    MPI_Type_lb(type, &lb);
    MPI_Type_ub(type, &ub);
    printf("lb: %li, extent: %li, ub: %li \n", lb, extent, ub);

    MPI_Type_size(type, &size);
    MPI_Type_get_extent(type, &lb, &extent);
    MPI_Type_get_true_extent(type, &true_lb, &true_extent);

    printf("lb: %li, extent: %li, ub: %li \n", lb, extent, lb + extent);
    printf("true_lb: %li, true_extent: %li, true_ub: %li \n", true_lb, true_extent, true_lb + true_extent);
    printf("size: %i\n", size);
#endif
}

void printIntMap(MPI_Datatype type)
{
#ifdef HAVE_MPI_TYPE_GET_TRUE_EXTENT
    int size, int_size, i, pack_size, position=0;
    MPI_Aint true_lb, true_extent, lb, extent;

    MPI_Type_size(type, &size);
    MPI_Type_size(MPI_INT, &int_size);
    MPI_Type_get_true_extent(type, &true_lb, &true_extent);
    MPI_Type_get_extent(type, &lb, &extent);
    MPI_Pack_size(1, type, MPI_COMM_SELF, &pack_size);

    MPI_Datatype serialConti;
    MPI_Type_contiguous(size/int_size, MPI_INT, &serialConti);
    MPI_Type_commit(&serialConti);

    int *inbuf = malloc(sizeof(char)*true_extent);
    int *outbuf = malloc(sizeof(char)*size);
    char *packbuff = malloc(sizeof(char)*pack_size);

    for (i=0; i < true_extent / int_size; i++)
        inbuf[i] = i + true_lb / int_size;
    MPI_Pack(inbuf - true_lb / int_size, 1, type, packbuff, pack_size, &position, MPI_COMM_SELF);
    position=0;
    MPI_Unpack(packbuff, pack_size, &position, outbuf, 1, serialConti, MPI_COMM_SELF);
    for (i=0; i < size/int_size; i++)
        printf("(MPI_INT, %i), ", outbuf[i]);
    printf("\b\b  \r\n\r\n");
    MPI_Fint ftype = MPI_Type_c2f(type);
    MPI_Initialized (&ftype);
    MPI_Type_free(&serialConti);
#endif
}

void noopTest(MPI_Datatype type){}

void advancedTest(MPI_Datatype type)
{
    MPI_Datatype contitype;
    int i;
    for (i=1; i<8; i+=6)
    {
        MPI_Type_contiguous (i, type, &contitype);
        MPI_Type_commit (&contitype);
        printTypeAttributes(contitype);
        printIntMap(contitype);
        MPI_Type_free (&contitype);
    }
}

int mpiResizedByStruct(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype* newtype)
{
    int array_of_blocklengths[] = {0, 1, 0};
    MPI_Aint array_of_displacements[] = {lb, 0, lb+extent};
    MPI_Datatype array_of_types[] = {MPI_LB, oldtype, MPI_UB};
    return MPI_Type_struct(3, array_of_blocklengths, array_of_displacements, array_of_types, newtype);
}

int mpiResizedByStructLimits(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype* newtype)
{
    int array_of_blocklengths[] = {1, 1, 1};
    MPI_Aint array_of_displacements[] = {lb,0,lb+extent};
    MPI_Datatype array_of_types[] = {MPI_LB,oldtype,MPI_UB};
    return MPI_Type_struct(3, array_of_blocklengths, array_of_displacements, array_of_types, newtype);
}

int mpiResizedByHindexed(MPI_Datatype oldtype, MPI_Aint lb, MPI_Aint extent, MPI_Datatype* newtype)
{
    int array_of_blocklengths[] = {0, 1, 0};
    MPI_Aint array_of_displacements[] = {lb, 0, lb+extent};
    return MPI_Type_hindexed(3, array_of_blocklengths, array_of_displacements, oldtype, newtype);
}

