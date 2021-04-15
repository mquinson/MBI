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
 * @file BufferChecks.h
 *       @see MUST::BufferChecks.
 *
 *  @date 11.01.2013
 *  @author Joachim Protze
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"

#include "I_BufferChecks.h"

#include <string>

#ifndef BUFFERCHECKS_H
#define BUFFERCHECKS_H

using namespace gti;

namespace must
{
    /**
     * BufferChecks for correctness checks interface implementation.
     */
    class BufferChecks : public gti::ModuleBase<BufferChecks, I_BufferChecks>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
            BufferChecks (const char* instanceName);

            /**
             * Destructor.
             */
            virtual ~BufferChecks (void);

            /**
             * @see I_BufferChecks::bufferAttach.
             */
    GTI_ANALYSIS_RETURN bufferAttach (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            int size);

            /**
             * @see I_BufferChecks::bufferDetach.
             */
    GTI_ANALYSIS_RETURN bufferDetach (
            MustParallelId pId, 
            MustLocationId lId);

            /**
             * @see I_BufferChecks::bufferUsage.
             */
    GTI_ANALYSIS_RETURN bufferUsage (
            MustParallelId pId, 
            MustLocationId lId,
            int size);

    protected:
            I_ParallelIdAnalysis* myPIdMod;
            I_CreateMessage* myLogger;
            I_ArgumentAnalysis* myArgMod;
            
            int bufferSize, bufferLoad;
    };
} /*namespace MUST*/

#endif /*BUFFERCHECKS_H*/
