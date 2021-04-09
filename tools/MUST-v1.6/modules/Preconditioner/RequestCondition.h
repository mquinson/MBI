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
 * @file RequestCondition.h
 *       @see MUST::RequestCondition.
 *
 *  @date 06.06.2011
 *  @author Joachim Protze
 */

#include "ModuleBase.h"

#include "I_RequestCondition.h"

#include <string>

#ifndef REQUESTCONDITION_H
#define REQUESTCONDITION_H

using namespace gti;

namespace must
{
	/**
     * Template for correctness checks interface implementation.
     */
    class RequestCondition : public gti::ModuleBase<RequestCondition, I_RequestCondition>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		RequestCondition (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~RequestCondition (void);

    		/**
    		 * @see I_RequestCondition::addPredefineds
    		 */
    		GTI_ANALYSIS_RETURN addPredefineds (
    				int anySource);

    		/**
    		 * @see I_RequestCondition::complete
    		 */
    		GTI_ANALYSIS_RETURN complete (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType request,
    				int flag,
    				int statusSource);

    		/**
    		 * @see I_RequestCondition::completeAny
    		 */
    		GTI_ANALYSIS_RETURN completeAny (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType* requests,
    				int count,
    				int index,
    				int flag,
    				int statusSource);

    		/**
    		 * @see I_RequestCondition::completeArray
    		 */
    		GTI_ANALYSIS_RETURN completeArray (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType *requests,
    				int count,
    				int flag,
    				int* statusSources);

    		/**
    		 * @see I_RequestCondition::completeSome
    		 */
    		GTI_ANALYSIS_RETURN completeSome (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustRequestType *requests,
    				int count,
    				int *indices,
    				int numIndices,
    				int* statusSources);

    protected:
    		int myAnySource;

    		propagateRequestRealCompleteP myPComplete;
    		propagateRequestsRealCompleteP myPCompletes;
    };
} /*namespace MUST*/

#endif /* REQUESTCONDITION_H */
