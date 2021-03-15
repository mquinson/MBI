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
 * @file I_InitParallelId.h
 *       Interface to set a parallel Id.
 *
 *  @date 16.04.2014
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "BaseIds.h"

#ifndef I_INITPARALLELID_H
#define I_INITPARALLELID_H

/**
 * Interface for an integrity analysis that can initialize a parallel ID.
 */
class I_InitParallelId : public gti::I_Module
{
public:
    /**
     * Sets the given storage to a parallel identifier for this application process/thread/?.
     * (What influences the parallel ID depends on what paradigm we are talking about)
     * @param pStorage pointer to the storage to which we will store the created parallel ID.
     * @return @see GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN init (MustParallelId *pStorage) = 0;

}; /*class I_InitParallelId*/

#endif /*I_INITPARALLELID_H*/
