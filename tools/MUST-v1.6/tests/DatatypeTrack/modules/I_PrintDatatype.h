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
 * @file I_PrintDatatype.h
 *       @see I_PrintDatatype.
 *
 *  @date 23.02.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h" //TODO Needs to be renamed to GTI enums
#include "BaseIds.h"
#include "I_DatatypeTrack.h"

#ifndef I_PRINTDATATYPE_H
#define I_PRINTDATATYPE_H

/**
 * Interface for printing information on a datatype.
 *
 * Needs two sub modules, using the following order:
 * - I_DatatypeTrack
 * - I_CreateMessage
 */
class I_PrintDatatype : public gti::I_Module
{
public:
	/**
	 * Prints information on the given datatype.
	 * @param pId of the call site.
	 * @param lId of the call site.
	 * @param type to print.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN print (
    		MustParallelId pId,
    		MustLocationId lId,
    		MustDatatypeType type) = 0;

}; /*class I_PrintDatatype*/

#endif /*I_PRINTDATATYPE_H*/
