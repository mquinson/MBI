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
 * @file I_ThreadSanity.h
 *       @see I_ThreadSanity.
 *
 *  @date 31.01.2014
 *  @author Felix Muenchhalfen
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifdef GTI_OMPT_FOUND
#include <ompt.h>
#endif

#ifndef I_THREADSANITY_H
#define I_THREADSANITY_H

/**
 * Interface for correctness checks for threads.
 *
 * Dependencies (order as listed):
 * CreateMessage
 * LocationIdInit
 * ParallelIdInit
 *
 */
class I_ThreadSanity : public gti::I_Module
{
public:

virtual gti::GTI_ANALYSIS_RETURN notifyInitThreaded (
                MustParallelId pId,
                MustLocationId lId,
                int provided )=0;

#ifdef GTI_OMPT_FOUND
virtual gti::GTI_ANALYSIS_RETURN notifyParallelBegin (
                ompt_task_id_t parent_task_id,
                ompt_frame_t *parent_task_frame,
                ompt_parallel_id_t parallel_id,
                uint32_t requested_team_size,
                void *parallel_function )=0;

virtual gti::GTI_ANALYSIS_RETURN notifyParallelEnd (
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyBarrierBegin (
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyBarrierEnd (
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyBarrierWaitBegin (
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyBarrierWaitEnd (
                ompt_parallel_id_t parallel_id,
                ompt_task_id_t task_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyInitLock (
                ompt_wait_id_t wait_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyInitNestLock (
                ompt_wait_id_t wait_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyAcquiredLock (
                ompt_wait_id_t wait_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyWaitLock (
                ompt_wait_id_t wait_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyAcquiredNestLock (
                ompt_wait_id_t wait_id)=0;

virtual gti::GTI_ANALYSIS_RETURN notifyWaitNestLock (
                ompt_wait_id_t wait_id)=0;
#endif

virtual gti::GTI_ANALYSIS_RETURN enterMPICall (MustParallelId pId, MustLocationId lId) = 0;
virtual gti::GTI_ANALYSIS_RETURN leaveMPICall (MustParallelId pId, MustLocationId lId) = 0;

};

#endif /*I_THREADSANITY_H*/
