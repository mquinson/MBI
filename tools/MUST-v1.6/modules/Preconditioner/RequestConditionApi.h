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
 * @file RequestConditionApi.h
 *      P call definition for MUST preconditioned Request calls.
 *
 * @author Joachim Protze
 * @date 06.06.2011
 */

#include "I_RequestTrack.h"

#ifndef REQUESTCONDITIONAPI_H
#define REQUESTCONDITIONAPI_H

//==Function used for propagating non-blocking receive wildcard updates
inline int PpropagateRequestRealComplete (
        MustParallelId pId,
        MustLocationId lId,
        int source,
        MustRequestType request)  {return 0;}

typedef int (*propagateRequestRealCompleteP) (
        MustParallelId pId,
        MustLocationId lId,
        int source,
        MustRequestType request);

//==Function used for propagating non-blocking receive wildcard updates
inline int PpropagateRequestsRealComplete (
        MustParallelId pId,
        MustLocationId lId,
        int *sources,
        MustRequestType* requests,
        int count)  {return 0;}

typedef int (*propagateRequestsRealCompleteP) (
        MustParallelId pId,
        MustLocationId lId,
        int *sources,
        MustRequestType* requests,
        int count);

#endif /* REQUESTCONDITIONAPI_H */
