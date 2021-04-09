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
 * @file I_ParallelIdAnalysis.h
 *       Interface for the location module.
 *
 *  @date 07.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "ModuleBase.h"

#ifndef I_PARALLELIDANALYSIS_H
#define I_PARALLELIDANALYSIS_H

/**
 * Information for a parallel id.
 */
struct ParallelInfo
{
	int rank; /**< Rank of the process.*/
        int threadid;
};

/**
 * Parallel id analysis interface.
 */
class I_ParallelIdAnalysis : public gti::I_Module
{
public:
    /**
     * Returns information on the given id.
     * @param id to get parallel identifier information for.
     * @return information for this parallel id.
     */
    virtual ParallelInfo getInfoForId (MustParallelId id) = 0;

    /**
     * Returns a textual description of the parallel id.
     * @param id parallel id.
     * @return textual representation.
     */
    virtual std::string toString (MustParallelId id) = 0;

}; /*class I_ParallelIdAnalysis*/

#endif /*I_PARALLELIDANALYSIS_H*/
