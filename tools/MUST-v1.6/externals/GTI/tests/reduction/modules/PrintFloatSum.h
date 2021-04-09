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
 * @file PrintFloatSum.h
 *       Interface for the reduction example print module.
 *
 *  @date 23.12.2010
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "I_ChannelId.h"

#ifndef PRINTFLOATSUM_H
#define PRINTFLOATSUM_H

/** PrintFloatSum interface. */
class I_PrintFloatSum : public gti::I_Module
{
public:
	/** Performs the printing. */
    virtual gti::GTI_ANALYSIS_RETURN print (float f, gti::I_ChannelId* id) = 0;
}; /*class I_PrintFloatSum*/

#endif /*PRINTFLOATSUM_H*/
