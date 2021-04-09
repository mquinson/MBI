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
 * @file I_DWaitStateCollReduction.h
 *       @see I_DWaitStateCollReduction.
 *
 *  @date 05.03.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#include "I_ChannelId.h"
#include "I_Reduction.h"

#ifndef I_DWAITSTATECOLLREDUCTION_H
#define I_DWAITSTATECOLLREDUCTION_H

/**
 * Listens to collectiveActive requests and acknowledges them
 * if DWaitStateCollReduction determines that a request is complete.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CommTrack
 * - DCollectiveMatchReduction
 *
 */
class I_DWaitStateCollReduction : public gti::I_Module, public gti::I_Reduction
{
public:

    /**
     * Notification of a new collectiveActive request.
     * This filter out ALL incoming requests, and only
     * creates a new event if the request is complete,
     * since we do not care about event order here, since
     * DWaitState will never create two requests involving
     * the same task before it gets an acknowledge.
     *
     * Also as a result timeouts are of no interest to us,
     * we succeed or not, irrespective of timing.
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param comm of the request.
     * @param numTasks in the aggregated request.
     * @param cId @see gti::I_Reduction.
     * @param outFinishedChannels @see gti::I_Reduction.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN request (
            int isIntercomm,
            unsigned long long contextId,
            int CollCommType,
            int localGroupSize,
            int remoteGroupSize,
            int numTasks,
            gti::I_ChannelId *cId,
            std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

};/*class I_DWaitStateCollReduction*/

#endif /*I_DWAITSTATECOLLREDUCTION_H*/
