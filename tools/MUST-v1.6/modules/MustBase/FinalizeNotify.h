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
 * @file FinalizeNotify.h
 *       @see MUST::FinalizeNotify.
 *
 *  @date 04.04.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"

#include "I_FinalizeNotify.h"

#ifndef FINALIZENOTIFY_H
#define FINALIZENOTIFY_H

using namespace gti;

namespace must
{
	/**
     * Template for correctness checks interface implementation.
     */
    class FinalizeNotify : public gti::ModuleBase<FinalizeNotify, I_FinalizeNotify>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		FinalizeNotify (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~FinalizeNotify (void);

    		/**
    		 * @see I_FinalizeNotify::notify.
    		 */
    		GTI_ANALYSIS_RETURN notify ();

    protected:
    };
} /*namespace MUST*/

#endif /*FINALIZENOTIFY_H*/
