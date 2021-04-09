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
 * @file ProfilerMpi.h
 *       @see MUST::ProfilerMpi.
 *
 *  @date 16.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "ModuleBase.h"

#include "I_Profiler.h"

#include <map>

#ifndef PROFILERMPI_H
#define PROFILERMPI_H

namespace gti
{
    /**
     * Implementation of I_PanicHandler.
     */
    class ProfilerMpi : public ModuleBase<ProfilerMpi, I_Profiler>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        ProfilerMpi (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~ProfilerMpi (void);

        /**
         * @see I_Profiler::reportIdleTime
         */
        GTI_ANALYSIS_RETURN reportIdleTime (uint64_t usecIdle);

        /**
         * @see I_Profiler::reportMaxBadness
         */
        GTI_ANALYSIS_RETURN reportMaxBadness (uint64_t badness);

        /**
         * @see I_Profiler::reportTimeoutTime
         */
        GTI_ANALYSIS_RETURN reportTimeoutTime (uint64_t usec, uint64_t count);

        /**
         * @see I_Profiler::reportDownCommTime
         */
        GTI_ANALYSIS_RETURN reportDownCommTime (uint64_t usecDown, uint64_t count);

        /**
         * @see I_Profiler::reportUpCommTime
         */
        GTI_ANALYSIS_RETURN reportUpCommTime (uint64_t usecUp, uint64_t count);

        /**
         * @see I_Profiler::reportIntraCommTime
         */
        GTI_ANALYSIS_RETURN reportIntraCommTime (uint64_t usecIntra, uint64_t count);

        /**
         * @see I_Profiler::reportWrapperAnalysisTime
         */
        GTI_ANALYSIS_RETURN reportWrapperAnalysisTime (std::string moduleName, std::string analysisName, uint64_t time, uint64_t count);

        /**
         * @see I_Profiler::reportAnalysisTime
         */
        GTI_ANALYSIS_RETURN reportReceivalAnalysisTime (std::string moduleName, std::string analysisName, uint64_t time, uint64_t count);

        /**
         * @see I_Profiler::setWrapperEntryTime
         */
        GTI_ANALYSIS_RETURN setWrapperEntryTime (uint64_t usecTimeStamp);

        /**
         * @see I_Profiler::getLastWrapperEntryTime
         */
        uint64_t getLastWrapperEntryTime (void);

        /**
         * Creates the report.
         */
        void report (void);

    protected:
        /*all in usec*/
        uint64_t
            myStartTime,
            myIdleTime,
            myInfrastructureTime,
            myLastWrapperEntryTime,
            myMaxBadness; //Last is a count, not a usec

        typedef std::pair<uint64_t, uint64_t> FunctionInfo; //usec, invocation count
        typedef std::pair<FunctionInfo, FunctionInfo> TimePair; //first function info is wrapper, second is receival
        typedef std::map<std::string, TimePair> FunctionMap;
        typedef std::map<std::string, FunctionMap> ModuleMap;
        ModuleMap myAnalysisTimes;

        FunctionInfo
            myDownTime,
            myUpTime,
            myIntraTime,
            myTimeoutTime;
    };
} /*namespace gti*/

#endif /*PROFILERMPI_H*/
