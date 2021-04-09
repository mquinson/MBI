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
 * @file FinalizeMust.h
 *       @see MUST::FinalizeMust.
 *
 *  @date 05.03.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "BaseApi.h"

#include "I_FinalizeMust.h"

#ifndef FINALIZEMUST_H
#define FINALIZEMUST_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_FinalizeMust.
     */
    class FinalizeMust : public gti::ModuleBase<FinalizeMust, I_FinalizeMust>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		FinalizeMust (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~FinalizeMust (void);

    		/**
    		 * @see I_FinalizeMust::notify.
    		 */
    		GTI_ANALYSIS_RETURN notify ();

    protected:
    };
} /*namespace MUST*/

#endif /*FINALIZEMUST_H*/
