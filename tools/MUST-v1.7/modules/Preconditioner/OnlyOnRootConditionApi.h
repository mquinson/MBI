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
 * @file OnlyOnRootConditionApi.h
 *      Functions used as API calls for the "only on root" preconditioner @see I_OnlyOnRootCondition.
 *
 * @author Mathias Korepkat, Tobias Hilbrich, Joachim Protze
 * @date 23.08.2011
 */

#include "MustEnums.h"
#include "MustTypes.h"
#include "I_DatatypeTrack.h"
#include "I_CommTrack.h"
#include <iostream>

#ifndef ONLYONROOTCONDITIONAPI_H
#define ONLYONROOTCONDITIONAPI_H

//==Gatherv/Scatterv recv/send with counts and displs
inline int PMustOnRootTransferCounts (
        MustParallelId pId,
        MustLocationId lId,
        int isSend, /*True if send transfer, false otherwise*/
        MustAddressType buf,
        MustArgumentId bufArgId,
        const int* counts,
        MustArgumentId countsArgId,
        const int* displs,
        MustArgumentId displsArgId,
        MustDatatypeType type,
        MustArgumentId typeArgId,
        int commSize
)  {return 0;}

typedef int (*MustOnRootTransferCountsP) (
        MustParallelId pId,
        MustLocationId lId,
        int isSend, /*True if send transfer, false otherwise*/
        MustAddressType buf,
        MustArgumentId bufArgId,
        const int* counts,
        MustArgumentId countsArgId,
        const int* displs,
        MustArgumentId displsArgId,
        MustDatatypeType type,
        MustArgumentId typeArgId,
        int commSize
);

//==Gather/Scatter recv/send.
inline int PMustOnRootTransfer (
        MustParallelId pId,
        MustLocationId lId,
        int isSend, /*True if send transfer, false otherwise*/
        MustAddressType buf,
        MustArgumentId bufArgId,
        int count,
        MustArgumentId countArgId,
        MustDatatypeType type,
        MustArgumentId typeArgId
)  {return 0;}

typedef int (*MustOnRootTransferP) (
        MustParallelId pId,
        MustLocationId lId,
        int isSend, /*True if send transfer, false otherwise*/
        MustAddressType buf,
        MustArgumentId bufArgId,
        int count,
        MustArgumentId countArgId,
        MustDatatypeType type,
        MustArgumentId typeArgId
);

#endif /* ONLYONROOTCONDITIONAPI_H */
