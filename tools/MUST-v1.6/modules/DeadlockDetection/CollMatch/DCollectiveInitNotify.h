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
 * @file DCollectiveInitNotify.h
 *       @see MUST::DCollectiveInitNotify.
 *
 *  @date 03.05.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#include "ModuleBase.h"
#include "I_DCollectiveInitNotify.h"

#ifndef DCOLLECTIVEINITNOTIFY_H
#define DCOLLECTIVEINITNOTIFY_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_DCollectiveInitNotify.
     */
    class DCollectiveInitNotify : public gti::ModuleBase<DCollectiveInitNotify, I_DCollectiveInitNotify>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DCollectiveInitNotify (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~DCollectiveInitNotify (void);

    		/**
    		 * @see I_DCollectiveInitNotify::notifyInit
    		 */
    		GTI_ANALYSIS_RETURN notifyInit (void);

    }; /*class DCollectiveInitNotify */
} /*namespace MUST*/

#endif /*DCOLLECTIVEINITNOTIFY_H*/
