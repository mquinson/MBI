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
 * @file CollectiveCondition.h
 *       @see MUST::CollectiveCondition.
 *
 *  @date 06.06.2011
 *  @author Joachim Protze
 */

#include "ModuleBase.h"

#include "I_ParallelIdAnalysis.h"
#include "I_CollectiveCondition.h"
#include "I_CommTrack.h"
#include "I_GroupTable.h"

#include <string>

#ifndef COLLECTIVECONDITION_H
#define COLLECTIVECONDITION_H

using namespace gti;

namespace must
{
	/**
     * Template for correctness checks interface implementation.
     */
    class CollectiveCondition : public gti::ModuleBase<CollectiveCondition, I_CollectiveCondition>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		CollectiveCondition (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~CollectiveCondition (void);

    		/**
    		 * @see I_CollectiveCondition::noTransfer.
    		 */
    		GTI_ANALYSIS_RETURN noTransfer (
    		        MustParallelId pId,
    		        MustLocationId lId,
    		        int coll, // formerly gti::MustCollCommType
    		        MustCommType comm,
    	            int hasRequest,
    	            MustRequestType request
    		);

    		/**
    		 * @see I_CollectiveCondition::gather
    		 */
        GTI_ANALYSIS_RETURN gather (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                int sendcount,
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                int recvcount,
                MustDatatypeType recvtype,
                int root,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::gatherv
             */
        GTI_ANALYSIS_RETURN gatherv (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                int sendcount,
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                const int recvcounts[],
                const int displs[],
                MustDatatypeType recvtype,
                int root,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::reduce
             */
        GTI_ANALYSIS_RETURN reduce (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustAddressType recvbuf,
                int count,
                MustDatatypeType datatype,
                MustOpType op,
                int root,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::bcast
             */
        GTI_ANALYSIS_RETURN bcast (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType buffer,
                int count,
                MustDatatypeType datatype,
                int root,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::scatter
             */
        GTI_ANALYSIS_RETURN scatter (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                int sendcount,
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                int recvcount,
                MustDatatypeType recvtype,
                int root,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::scatterv
             */
        GTI_ANALYSIS_RETURN scatterv (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                const int sendcounts[],
                const int displs[],
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                int recvcount,
                MustDatatypeType recvtype,
                int root,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::allgather
             */
        GTI_ANALYSIS_RETURN allgather (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                int sendcount,
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                int recvcount,
                MustDatatypeType recvtype,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::allgatherv
             */
        GTI_ANALYSIS_RETURN allgatherv (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                int sendcount,
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                const int recvcounts[],
                const int displs[],
                MustDatatypeType recvtype,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::alltoall
             */
        GTI_ANALYSIS_RETURN alltoall (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                int sendcount,
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                int recvcount,
                MustDatatypeType recvtype,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::alltoallv
             */
        GTI_ANALYSIS_RETURN alltoallv (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                const int sendcounts[],
                const int sdispls[],
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                const int recvcounts[],
                const int rdispls[],
                MustDatatypeType recvtype,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::alltoallw
             */
        GTI_ANALYSIS_RETURN alltoallw (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                const int sendcounts[],
                const int sdispls[],
                const MustDatatypeType sendtypes[],
                MustAddressType recvbuf,
                const int recvcounts[],
                const int rdispls[],
                const MustDatatypeType recvtypes[],
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::allreduce
             */
        GTI_ANALYSIS_RETURN allreduce (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustAddressType recvbuf,
                int count,
                MustDatatypeType datatype,
                MustOpType op,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::reduce_scatter
             */
        GTI_ANALYSIS_RETURN reduce_scatter (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustAddressType recvbuf,
                const int recvcounts[],
                MustDatatypeType datatype,
                MustOpType op,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::reduce_scatter_block
             */
        GTI_ANALYSIS_RETURN reduce_scatter_block (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustAddressType recvbuf,
                int recvcount,
                MustDatatypeType datatype,
                MustOpType op,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::scan
             */
        GTI_ANALYSIS_RETURN scan (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustAddressType recvbuf,
                int count,
                MustDatatypeType datatype,
                MustOpType op,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

            /**
             * @see I_CollectiveCondition::exscan
             */
        GTI_ANALYSIS_RETURN exscan (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                MustAddressType recvbuf,
                int count,
                MustDatatypeType datatype,
                MustOpType op,
                MustCommType comm,
                int hasRequest,
                MustRequestType request
                );

        protected:
            I_ParallelIdAnalysis* myPIdMod;
            I_CommTrack* myComMod;
            /* simplify your life */
            int pId2Rank(
                    MustParallelId pId);

            Must_Coll_No_TransferP myPNoTransfer;
            Must_Coll_SendP myPSend;
            Must_Coll_Op_SendP myPOpSend;
            Must_Coll_Send_nP myPSendN;
            Must_Coll_Op_Send_nP myPOpSendN;
            Must_Coll_Send_buffersP myPSendBuffers;
            Must_Coll_Op_Send_buffersP myPOpSendBuffers;
            Must_Coll_Send_countsP myPSendCounts;
            Must_Coll_Op_Send_countsP myPOpSendCounts;
            Must_Coll_Send_typesP myPSendTypes;
            Must_Coll_RecvP myPRecv;
            Must_Coll_Recv_nP myPRecvN;
            Must_Coll_Op_Recv_nP myPOpRecvN;
            Must_Coll_Recv_buffersP myPRecvBuffers;
            Must_Coll_Recv_countsP myPRecvCounts;
            Must_Coll_Recv_typesP myPRecvTypes;
            Must_Coll_Send_RecvP myPSendRecv;
    };
} /*namespace MUST*/

#endif /*TEMPLATE_H*/
