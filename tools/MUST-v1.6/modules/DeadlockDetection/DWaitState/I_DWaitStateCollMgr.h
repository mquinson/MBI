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
 * @file I_DWaitStateCollMgr.h
 *       @see I_DWaitStateCollMgr.
 *
 *  @date 05.03.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#include "I_ChannelId.h"

#ifndef I_DWAITSTATECOLLMGR_H
#define I_DWAITSTATECOLLMGR_H

/**
 * Listens to collectiveActive requests and acknowledges them
 * if DWaitStateCollReduction determines that a request is complete.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CommTrack
 *
 */
class I_DWaitStateCollMgr : public gti::I_Module
{
public:

    /**
     * Notification of an complete collectiveActive request.
     * The way DWaitStateCollReduction works cId must be
     * NULL, i.e., it must be a complete wave, if it is not,
     * then an internal error exists.
     *
     * @param pId parallel id of the call site.
     * @param lId location id of the call site.
     * @param comm of the request.
     * @param numTasks in the aggregated request.
     * @param cId of the request.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN request (
            int isIntercomm,
            unsigned long long contextId,
            int collCommType,
            int localGroupSize,
            int remoteGroupSize,
            int numTasks,
            gti::I_ChannelId* cId) = 0;

};/*class I_DWaitStateCollMgr*/

#endif /*I_DWAITSTATECOLLMGR_H*/
