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
 * @file I_PrintErr.h
 *       @see I_PrintErr.
 *
 *  @date 12.05.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h" //TODO Needs to be renamed to GTI enums
#include "BaseIds.h"
#include "I_ErrTrack.h"
#include "I_LocationAnalysis.h"

#ifndef I_PRINTERR_H
#define I_PRINTERR_H

/**
 * Interface for a module that prints all available
 * information on an errorhandler, using I_ErrTrack.
 *
 * Dependencies:
 *  1) LocationAnalysis
 *  2) CreateMessage
 *  3) ErrTrack
 */
class I_PrintErr : public gti::I_Module
{
public:
	/**
	 * Prints information on the given err as log events.
	 * @param pId parallel Id of the print call.
	 * @param lId location Id of the print call.
	 * @param err to print.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN print (
    		MustParallelId pId,
    		MustLocationId lId,
    		MustErrType err) = 0;

}; /*class I_PrintErr*/

#endif /*I_PRINTERR_H*/
