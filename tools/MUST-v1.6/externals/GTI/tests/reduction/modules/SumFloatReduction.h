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
 * @file SumFloatReduction.h
 *       Interface for the reduction of a reduction example.
 *
 *  @date 23.12.2010
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "I_Reduction.h"
#include "I_ChannelId.h"
#include "GtiEnums.h"

#include <list>

#ifndef SUMFLOATREDUCTION_H
#define SUMFLOATREDUCTION_H

/** SummAllFloats interface. */
class I_SumFloatReduction : public gti::I_Module, public gti::I_Reduction
{
public:
	/** Performs the reduction. */
    virtual gti::GTI_ANALYSIS_RETURN reduce (float f, gti::I_ChannelId *thisChannel, std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;
}; /*class I_SumFloatReduction*/

#endif /*SUMFLOATREDUCTION_H*/
