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
 * @file I_ArgumentAnalysis.h
 *       @see I_ArgumentAnalysis.
 *
 *  @date 28.02.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "BaseIds.h"

#ifndef I_ARGUMENTANALYSIS_H
#define I_ARGUMENTANALYSIS_H

/**
 * Interface for translating argument ids into an argument index and
 * an argument name.
 */
class I_ArgumentAnalysis : public gti::I_Module
{
public:
	/**
	 * Returns the index of the argument in its MPI call.
	 * Starts with 1 for the first argument not with 0.
	 * @param id to query for information.
	 * @return index of argument.
	 */
    virtual int getIndex (MustArgumentId id) = 0;

    /**
     * Returns the name of an MPI calls argument.
     * @param id to query for information.
     * @return name of argument.
     */
    virtual std::string getArgName (MustArgumentId id) = 0;

}; /*class I_ArgumentAnalysis*/

#endif /*I_ARGUMENTANALYSIS_H*/
