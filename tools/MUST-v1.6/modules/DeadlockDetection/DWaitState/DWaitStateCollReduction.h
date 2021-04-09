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
 * @file DWaitStateCollReduction.h
 *       @see DWaitStateCollReduction.
 *
 *  @date 05.03.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_CommTrack.h"
#include "I_DWaitStateCollReduction.h"
#include "I_CollCommListener.h"
#include "I_DCollectiveMatchReduction.h"
#include "DistributedDeadlockApi.h"
#include "CompletionTree.h"

#include <list>

#ifndef DWAITSTATECOLLREDUCTION_H
#define DWAITSTATECOLLREDUCTION_H

using namespace gti;

namespace must
{
    /**
     * Implementation of I_DWaitStateCollReduction.
     * @see I_DWaitStateCollReduction
     */
    class DWaitStateCollReduction
        : public gti::ModuleBase<DWaitStateCollReduction, I_DWaitStateCollReduction>, public I_CollCommListener
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DWaitStateCollReduction (const char* instanceName);

            /**
             * Destructor.
             */
        virtual ~DWaitStateCollReduction (void);

        /**
         * @see I_DWaitStateCollReduction::request
         */
        GTI_ANALYSIS_RETURN request (
                int isIntercomm,
                unsigned long long contextId,
                int collCommType,
                int localGroupSize,
                int remoteGroupSize,
                int numTasks,
                I_ChannelId *cId,
                std::list<I_ChannelId*> *outFinishedChannels);

        /**
         * @see I_CollCommListener::newCommInColl
         */
        void newCommInColl (
                MustParallelId pId,
                I_CommPersistent* comm);

        /**
         * The timeout function, see gti::I_Reduction::timeout
         */
        void timeout (void);

    protected:
        generateCollectiveActiveRequestP myFForward;
        I_ParallelIdAnalysis* myPIdMod;
        I_CommTrack *myCommTrack;
        I_DCollectiveMatchReduction *myCollMatch;

        class CommInfo
        {
            public:
                int isIntercomm;
                unsigned long long contextId;
                int localSize;
                int remoteSize;
                int numConnected;
                std::list<std::pair<int, CompletionTree*> > activeRequests; /**< Maps number of arrived participants to the completion tree of the request.*/
                I_ChannelId *tempCId; /**< For the use in myUnexpectedRequests below, this stores a copy of the channel id.*/

                CommInfo (void);
                CommInfo (const CommInfo& other);
                ~CommInfo (void);
        };

        std::list<CommInfo> myInfos;
        std::list<CommInfo> myUnexpectedRequests;
        bool myInUnexepctedTest;

        /**
         * Returns true if matching information for two communicators is equal.
         * Mimics I_Comm::compareComms implementation
         */
        bool compare (
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize,
                int isIntercomm2,
                unsigned long long contextId2,
                int localGroupSize2,
                int remoteGroupSize2);
    };
} /*namespace MUST*/

#endif /*DWAITSTATECOLLREDUCTION_H*/
