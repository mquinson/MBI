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
 * @file I_OnlyOnRootCondition.h
 *       @see I_OnlyOnRootCondition.
 *
 *  @date 23.08.2011
 *  @author Mathias Korepkat, Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"

#ifndef I_ONLYONROOTCONDITION_H
#define I_ONLYONROOTCONDITION_H

/**
 * Interface for providing wildcard receive updates
 * to the lost message detector (I_LostMessage.h).
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CommTrack
 */
class I_OnlyOnRootCondition : public gti::I_Module
{
public:

    /**
         * Preconditioner for MPI_Gather
         *
         * @param pId parallel id of the call site.
         * @param lId location id of the call site.
         * @param recvbuf buffer for communication (just root)
         * @param recvbufArgId argument Id of the recv buffer.
         * @param recvcount repetitions of recvtype (just root)
         * @param recvcountArgId argument Id of the recv count.
         * @param recvtype datatype for recv (just root)
         * @param recvtypeArgId argument Id of the recv type.
         * @param root collecting node
         * @param comm communicator for communication
         * @return @see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN gather (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType recvbuf,
                MustArgumentId recvbufArgId,
                int recvcount,
                MustArgumentId recvcountArgId,
                MustDatatypeType recvtype,
                MustArgumentId recvtypeArgId,
                int root,
                MustCommType comm
                ) = 0;

    /**
         * Preconditioner for MPI_Gatherv
         *
         * @param pId parallel id of the call site.
         * @param lId location id of the call site.
         * @param recvbuf buffer for communication (just root)
         * @param recvbufArgId argument Id of the recv buffer.
         * @param recvcounts[] repetitions of recvtype (just root)
         * @param recvcountsArgId argument Id of the recv counts.
         * @param displs[] displacements for recv (just root)
         * @param displsArgId argument Id of the displacements.
         * @param recvtype datatype for recv (just root)
         * @param recvtypeArgId argument Id of the recv type.
         * @param root collecting node
         * @param comm communicator for communication
         * @return @see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN gatherv (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType recvbuf,
                MustArgumentId recvbufArgId,
                const int recvcounts[],
                MustArgumentId recvcountsArgId,
                const int displs[],
                MustArgumentId displsArgId,
                MustDatatypeType recvtype,
                MustArgumentId recvtypeArgId,
                int root,
                MustCommType comm
                ) = 0;

    /**
         * Preconditioner for MPI_Scatter
         *
         * @param pId parallel id of the call site.
         * @param lId location id of the call site.
         * @param sendbuf buffer for communication (just root)
         * @param sendbufArgId argument Id of the send buffer.
         * @param sendcount repetitions of sendtype (just root)
         * @param sendcountArgId argument Id of the send count.
         * @param sendtype datatype for send (just root)
         * @param sendtypeArgId argument Id of the send datatype.
         * @param root distributing node
         * @param comm communicator for communication
         * @return @see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN scatter (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustArgumentId sendbufArgId,
                int sendcount,
                MustArgumentId sendcountArgId,
                MustDatatypeType sendtype,
                MustArgumentId sendtypeArgId,
                int root,
                MustCommType comm
                ) = 0;

        /**
        * Preconditioner for MPI_Scatterv
        *
        * @param pId parallel id of the call site.
        * @param lId location id of the call site.
        * @param sendbuf buffer for communication (just root)
        * @param sendbufArgId argument Id of the send buffer.
        * @param sendcount repetitions of sendtype (just root)
        * @param sendcountArgId argument Id of the send count.
        * @param displs[] displacements for send (just root)
        * @param displsArgId argument Id of the displacemen
        * @param sendtype datatype for send (just root)
        * @param sendtypeArgId argument Id of the send datatype.
        * @param root distributing node
        * @param comm communicator for communication
        * @return @see gti::GTI_ANALYSIS_RETURN.
        */
        virtual gti::GTI_ANALYSIS_RETURN scatterv (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustArgumentId sendbufArgId,
                const int sendcounts[],
                MustArgumentId sendcountsArgId,
                const int displs[],
                MustArgumentId displsArgId,
                MustDatatypeType sendtype,
                MustArgumentId sendtypeArgId,
                int root,
                MustCommType comm
                ) = 0;
};/*class I_OnlyOnRootCondition*/

#endif /*I_ONLYONROOT_H*/
