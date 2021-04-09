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
 * @file I_BufferChecks.h
 *       @see I_BufferChecks.
 *
 *  @date 11.01.2013
 *  @author Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_DatatypeTrack.h"

#ifndef I_BUFFERCHECKS_H
#define I_BUFFERCHECKS_H

/**
 * Interface for correctness checks of datatypes.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 *
 */
class I_BufferChecks : public gti::I_Module
{
public:

    /**
     * Saves the attached buffer size
     * Checks for previous attached buffer
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param size of attached buffer.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN bufferAttach (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            int size) = 0;

    /**
     * Sets attached buffer size to 0
     * Checks for previous attached buffer
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN bufferDetach (
            MustParallelId pId, 
            MustLocationId lId) = 0;

    /**
     * increases buffer usage
     * Checks for previous attached buffer
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param size of buffer usage.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN bufferUsage (
            MustParallelId pId, 
            MustLocationId lId,
            int size) = 0;


};/*class I_BufferChecks*/

#endif /*I_BUFFERCHECKS_H*/
