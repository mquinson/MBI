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
 * @file I_Template.h
 *       @see I_Template.
 *
 *  @date 01.03.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_TEMPLATE_H
#define I_TEMPLATE_H

/**
 * <INTERFACE DESCRIPTION>.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 * - <FURTHER DEPENDENCIES>
 *
 */
class I_Template : public gti::I_Module
{
public:

	/**
	 * <FUNCTION DESCRIPTION>
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * <FURTHER ARGUMENTS>
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN analysisFunction (MustParallelId pId, MustLocationId lId, int aId) = 0;
};/*class I_Template*/

#endif /*I_TEMPLATE_H*/
