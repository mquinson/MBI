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
 * @file DWaitStateCollMgr.h
 *       @see DWaitStateCollMgr.
 *
 *  @date 05.03.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_CommTrack.h"
#include "I_DWaitStateCollMgr.h"
#include "DistributedDeadlockApi.h"

#ifndef DWAITSTATECOLLMGR_H
#define DWAITSTATECOLLMGR_H

using namespace gti;

namespace must
{
    /**
     * Implementation of I_DWaitStateCollMgr.
     * @see I_DWaitStateCollMgr
     */
    class DWaitStateCollMgr
        : public gti::ModuleBase<DWaitStateCollMgr, I_DWaitStateCollMgr>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DWaitStateCollMgr (const char* instanceName);

            /**
             * Destructor.
             */
        virtual ~DWaitStateCollMgr (void);

        /**
         * @see I_DWaitStateCollMgr::request
         */
        GTI_ANALYSIS_RETURN request (
                int isIntercomm,
                unsigned long long contextId,
                int collCommType,
                int localGroupSize,
                int remoteGroupSize,
                int numTasks,
                I_ChannelId* cId);

    protected:
            generateCollectiveActiveAcknowledgeP myFAcknowledge;
            I_ParallelIdAnalysis* myPIdMod;
            I_CommTrack *myCommTrack;
    };
} /*namespace MUST*/

#endif /*DWAITSTATECOLLMGR_H*/
