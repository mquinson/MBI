/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file SimpleMod.h
 *       Interface for a set of simple modules, used by a test case.
 *
 *  @date 25.08.2010
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "I_Reduction.h"
#include "I_ChannelId.h"
#include "GtiEnums.h"

#include <list>

#ifndef SIMPLE_MOD_H
#define SIMPLE_MOD_H

/** SummAllFloats interface. */
class I_SumAllFloats : public gti::I_Module, public gti::I_Reduction
{
public:
	/** Performs the check. */
    virtual gti::GTI_ANALYSIS_RETURN analyse (float f, gti::I_ChannelId *thisChannel, std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;
}; /*class I_SumAllFloats*/

/** PrintFloatSum interface. */
class I_PrintFloatSum : public gti::I_Module
{
public:
	/** Performs the check. */
    virtual gti::GTI_ANALYSIS_RETURN analyse (float f) = 0;
}; /*class I_PrintFloatSum*/

/** CheckArray interface. */
class I_CheckArray : public gti::I_Module
{
public:
	/** Performs the check. */
	virtual gti::GTI_ANALYSIS_RETURN analyse (int, int*) = 0;
}; /*class I_CheckArray*/


/** HandleSizes interface. */
class I_HandleSizes : public gti::I_Module
{
public:
	/** Performs the check. */
    virtual gti::GTI_ANALYSIS_RETURN analyse (int, int*) = 0;

    /** Performs another check. */
    virtual gti::GTI_ANALYSIS_RETURN analyse2 (int) = 0;
}; /*class I_HandleSizes*/


/** SuperSumSize interface. */
class I_SuperSumSize : public gti::I_Module
{
public:
	/** Performs the check. */
    virtual gti::GTI_ANALYSIS_RETURN analyse (int) = 0;
}; /*class I_SuperSumSize*/


/** SumAllCounts interface. */
class I_SumAllCounts : public gti::I_Module
{
public:
	/** Performs the check. */
    virtual gti::GTI_ANALYSIS_RETURN analyse (int) = 0;
}; /*class I_SumAllCounts*/

#endif /*SIMPLE_MOD_H*/
