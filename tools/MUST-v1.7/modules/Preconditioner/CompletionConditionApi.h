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
 * @file CollectiveConditionApi.h
 *      P call definition for MUST preconditioned Request calls.
 *
 * @author Joachim Protze
 * @date 06.06.2011
 */

#include "MustEnums.h"
#include "MustTypes.h"

#ifndef COMPLETIONCONDITIONAPI_H
#define COMPLETIONCONDITIONAPI_H

//==Function used to forward a waitl completion that was pre-processed (which means filtered in this case)
inline int PpropagateReducedWait (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request
        )  {return 0;}

typedef int (*propagateReducedWaitP) (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request
        );

//==Function used to forward a waitall completion that was pre-processed
inline int PpropagateReducedWaitall (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count,
        int numProcNull /*Number of MPI_PROC_NULL associated requests that had been in the original array (they are not in this "requests" array)*/
        )  {return 0;}

typedef int (*propagateReducedWaitallP) (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count,
        int numProcNull
        );

//==Function used to forward a waitany completion that was pre-processed
inline int PpropagateReducedWaitany (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count,
        int numProcNull /*Number of MPI_PROC_NULL associated requests that had been in the original array (they are not in this "requests" array)*/
        )  {return 0;}

typedef int (*propagateReducedWaitanyP) (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count,
        int numProcNull
        );


//==Function used to forward a waitsome completion that was pre-processed
inline int PpropagateReducedWaitsome (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count,
        int numProcNull /*Number of MPI_PROC_NULL associated requests that had been in the original array (they are not in this "requests" array)*/
        )  {return 0;}

typedef int (*propagateReducedWaitsomeP) (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType *requests,
        int count,
        int numProcNull
        );


#endif /* COMPLETIONCONDITIONAPI_H */
