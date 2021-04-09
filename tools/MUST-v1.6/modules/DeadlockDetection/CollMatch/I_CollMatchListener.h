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
 * @file I_CollMatchListener.h
 *       @see I_CollMatchListener.
 *
 *  @date 10.08.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "BaseIds.h"
#include "MustEnums.h"
#include "MustTypes.h"
#include "I_Comm.h"

#ifndef I_COLLMATCHLISTENER_H
#define I_COLLMATCHLISTENER_H

namespace must
{
    /**
     * Interface to be implemented by listeners on collective matches.
     */
    class I_CollMatchListener
    {
    public:

        /**
         * Called when a new collective is completly matched.
         * @param comm only valid until this call returns.
         */
        virtual void newMatch (
                MustCollCommType collId,
                I_Comm* comm) = 0;

    };/*class I_CollMatchListener*/
} /*namespace must*/

#endif /*I_COLLMATCHLISTENER_H*/
