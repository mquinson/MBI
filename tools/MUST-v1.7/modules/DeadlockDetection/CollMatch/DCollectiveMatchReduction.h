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
 * @file DCollectiveMatchReduction.h
 *       @see must::DCollectiveMatchReduction.
 *
 *  @date 25.04.2012
 *  @author Fabian Haensel, Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#ifndef DCOLLECTIVEMATCHREDUCTION_H
#define DCOLLECTIVEMATCHREDUCTION_H

#include "ModuleBase.h"

#include "I_DCollectiveMatchReduction.h"
#include "DCollectiveMatch.h"

using namespace gti;

namespace must
{
	/**
     * Instantiation of distributed collective matching and verification.
     * This is the reduction part that matches and verifies across
     * all TBON layers.
     */
    class DCollectiveMatchReduction : public DCollectiveMatch<DCollectiveMatchReduction, I_DCollectiveMatchReduction>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DCollectiveMatchReduction (const char* instanceName);

		/**
		 * Destructor.
		 */
		virtual ~DCollectiveMatchReduction (void);

    };
} /*namespace MUST*/

#endif /*DCOLLECTIVEMATCHREDUCTION_H*/
