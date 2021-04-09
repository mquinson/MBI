/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file I_Profiler.h
 *       @see I_Profiler.
 *
 *  @date 16.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include <stdint.h>

#ifndef I_PROFILER_H
#define I_PROFILER_H

/**
 * Interface for profiling data.
 * Wrapper, Receival, and PlacementDriver should pass profiling information to this module.
 * In its destructor it should then store the profiling information in some format.
 *
 * Dependencies (order as listed):
 * - X
 */
class I_Profiler : public gti::I_Module
{
public:

    /**
     * Reports the total idle time to the profiler.
     * @param usecIdle time.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reportIdleTime (uint64_t usecIdle) = 0;

    /**
     * Reports the maximum badness that flood control perceived.
     * @param maximum badness.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reportMaxBadness (uint64_t badness) = 0;

    /**
     * Reports the total time spent for timeouts.
     * May be called multiple times, individual invocations of this call are summed.
     * @param usec time.
     * @param count invocation count of timeout.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reportTimeoutTime (uint64_t usec, uint64_t count) = 0;

    /**
     * Reports the total time spent to use the downwards connected
     * communication strategy.
     * May be called multiple times, individual invocations of this call are summed
     * @param usecDown time.
     * @param count invocation count of down communication.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reportDownCommTime (uint64_t usecDown, uint64_t count) = 0;

    /**
     * Reports the total time spent to use any upwards connected
     * communication strategy.
     * May be called multiple times, individual invocations of this call are summed.
     * @param usecUp time.
     * @param count invocation count of up communication.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reportUpCommTime (uint64_t usecUp, uint64_t count) = 0;

    /**
     * Reports the total time spent to use the intra
     * communication strategy [if present, assumed default tie is 0 usec].
     * May be called multiple times, individual invocations of this call are summed.
     * @param usecIntra time.
     * @param count invocation count of intra communication.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reportIntraCommTime (uint64_t usecIntra, uint64_t count) = 0;

    /**
     * Reports the total time for a single analysis function of a certain analysis module.
     * Used for time spent within the wrapper module for this analysis function.
     * @param moduleName name of the module.
     * @param analysisName name of the analysis.
     * @param time spent.
     * @param count invocation count of this function.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reportWrapperAnalysisTime (std::string moduleName, std::string analysisName, uint64_t time, uint64_t count) = 0;

    /**
     * Reports the total time for a single analysis function of a certain analysis module.
     * Used for time spent within the receival module for this analysis function.
     * @param moduleName name of the module.
     * @param analysisName name of the analysis.
     * @param time spent.
     * @param count invocation count of this function.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN reportReceivalAnalysisTime (std::string moduleName, std::string analysisName, uint64_t time, uint64_t count) = 0;

    /**
     * Notification to the profiler that a wrapper function was entered.
     * Can be used by other modules to determine whether some activity was interrupted
     * by entering a wrapper function.
     * @param usecTimeStamp time stamp when the wrapper was entered.
     */
    virtual gti::GTI_ANALYSIS_RETURN setWrapperEntryTime (uint64_t usecTimeStamp) = 0;

    /**
     * Returns the last wrapper entry time.
     * @return last entry time stamp.
     */
    virtual uint64_t getLastWrapperEntryTime (void) = 0;

};/*class I_Profiler*/

#endif /*I_PROFILER_H*/
