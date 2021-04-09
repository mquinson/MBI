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
 *  @date 21.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h" //TODO Needs to be renamed to GTI enums

#ifndef I_TEMPLATE_H
#define I_TEMPLATE_H

/**
 * Template interface for an analysis.
 */
class I_Template : public gti::I_Module
{
public:
	/**
	 *
	 */
    virtual gti::GTI_ANALYSIS_RETURN analysisFunction (void) = 0;

}; /*class I_Template*/

#endif /*I_TEMPLATE_H*/
