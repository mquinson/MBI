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
 * @file I_InitLocationId.h
 *       Interface to set a location Id.
 *
 *  @date 24.04.2014
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "BaseIds.h"

#ifndef I_INITLOCATIONID_H
#define I_INITLOCATIONID_H

/**
 * Interface for an integrity analysis that can initialize a location ID.
 *
 * Dependencies:
 * - InitParallelId
 */
class I_InitLocationId : public gti::I_Module
{
public:
    /**
     * Sets the given storage to a location identifier for the current event context, e.g., just a call name
     * or a full call stack.
     * @param callName name of the function call that created the event.
     * @param callId a globally unified number for this call name.
     * @param pStorage pointer to the storage to which we will store the created parallel ID.
     * @return @see GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN init (MustLocationId *pStorage, const char* callName, int callId) = 0;

}; /*class I_InitLocationId*/

#endif /*I_INITLOCATIONID_H*/
