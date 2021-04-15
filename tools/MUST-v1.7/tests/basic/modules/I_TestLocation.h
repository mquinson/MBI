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
 * @file I_TestLocation.h
 *       @see I_TestLocation.
 *
 *  @date 10.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "BaseIds.h"

#ifndef I_TESTLOCATION_H
#define I_TESTLOCATION_H

/**
 * Prints location and parallel id information.
 */
class I_TestLocation : public gti::I_Module
{
public:
	/**
	 * Prints location and parallel id.
	 * @param pId parallel Id for the location
	 * @param lId location id to print.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN print (MustParallelId pId, MustLocationId lId) = 0;

}; /*class I_TestLocation*/

#endif /*I_TESTLOCATION_H*/
