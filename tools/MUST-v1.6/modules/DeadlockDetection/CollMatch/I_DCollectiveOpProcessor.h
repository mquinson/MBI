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
 * @file I_DCollectiveOpProcessor.h
 *       @see I_DCollectiveOpProcessor.
 *
 *  @date 25.04.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#ifndef I_COLLECTIVEOPPROCESSOR_H
#define I_COLLECTIVEOPPROCESSOR_H

#include "BaseIds.h"
#include "GtiApi.h"
#include "DCollectiveOp.h"
#include "I_CreateMessage.h"
#include "I_LocationAnalysis.h"
#include "I_CommTrack.h"
#include "I_DatatypeTrack.h"
#include "DistributedDeadlockApi.h"
#include "CollectiveConditionApi.h"

namespace must
{
    /**
     * Forward declaration.
     */
    class DCollectiveOp;

    /**
     * Sub-interface for processing DCollectiveOp's.
     */
    class I_DCollectiveOpProcessor
    {
    public:
        /**
         * Translates a pId to a rank.
         */
        virtual int pIdToRank (MustParallelId pId) = 0;

        /**
         * Translates a rank in MPI_COMM_WORLD to the place id within the TBON
         * layer that receives events from this rank.
         * @param world rank to translate
         */
        virtual int getLevelIdForApplicationRank (int rank) = 0;

        /**
         * Provides a I_CreateMessage implementation.
         */
        virtual I_CreateMessage* getLogger (void) = 0;

        /**
         * Provides a I_DatatypeTrack implementation.
         */
        virtual I_DatatypeTrack* getDatatypeTrack (void) = 0;

        /**
         * Provides a I_CommTrack implementation.
         */
        virtual I_CommTrack* getCommTrack (void) = 0;

        /**
         * Provides a I_LocationAnalysis implementation.
         */
        virtual I_LocationAnalysis* getLocationModule (void) = 0;

        /**
         * Returns an array of size world size for temporary use.
         */
        virtual int* getWorldSizedCountArray (void) = 0;

        /**
         * Returns an size of comm world.
         */
        virtual int getWorldSize (void) = 0;

        /**
         * Functions to retrieve wrapper functions.
         */
        virtual Must_Coll_No_TransferP getNoTransferFct (void) = 0;
        virtual Must_Coll_SendP getSendFct (void) = 0;
        virtual Must_Coll_Op_SendP getOpSendFct (void) = 0;
        virtual Must_Coll_Send_nP getSendNFct (void) = 0;
        virtual Must_Coll_Op_Send_nP getOpSendNFct (void) = 0;
        virtual Must_Coll_Send_buffersP getSendBuffersFct (void) = 0;
        virtual Must_Coll_Op_Send_buffersP getOpSendBuffersFct (void) = 0;
        virtual Must_Coll_Send_countsP getSendCountsFct (void) = 0;
        virtual Must_Coll_Op_Send_countsP getOpSendCountsFct (void) = 0;
        virtual Must_Coll_Send_typesP getSendTypesFct (void) = 0;
        virtual Must_Coll_RecvP getRecvFct (void) = 0;
        virtual Must_Coll_Recv_nP getRecvNFct (void) = 0;
        virtual Must_Coll_Op_Recv_nP getOpRecvNFct (void) = 0;
        virtual Must_Coll_Recv_buffersP getRecvBuffersFct (void) = 0;
        virtual Must_Coll_Recv_countsP getRecvCountsFct (void) = 0;
        virtual Must_Coll_Recv_typesP getRecvTypesFct (void) = 0;

        /**
         * Functions to retrieve wrap across functions.
         */
        virtual passTypeMatchInfoP getPassTypeMatchInfoFct (void) = 0;
        virtual passTypeMatchInfoTypesP getPassTypeMatchInfoTypesFct (void) = 0;

        /**
         * Function to retrieve setNextEventStrided
         */
        virtual gtiSetNextEventStridedP getSetNextEventStridedFct (void) = 0;
    };
} //namespace must

#endif /*I_COLLECTIVEOPPROCESSOR_H*/
