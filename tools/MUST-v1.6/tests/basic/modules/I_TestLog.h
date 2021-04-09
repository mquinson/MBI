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
 *       @see I_TestLog.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "BaseIds.h"

#ifndef I_TESTLOG_H
#define I_TESTLOG_H

/**
 * Creates a logging message to test logging.
 */
class I_TestLog : public gti::I_Module
{
public:
	/**
	 * Creates a log event.
	 * @param pId parallel Id for the location
	 * @param lId location id to use for the message.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN test (MustParallelId pId, MustLocationId lId) = 0;

}; /*class I_TestLog*/

#endif /*I_TESTLOG_H*/
