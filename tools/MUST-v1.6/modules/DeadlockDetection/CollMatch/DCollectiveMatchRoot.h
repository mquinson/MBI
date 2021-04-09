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
 * @file DCollectiveMatchRoot.h
 *       @see must::DCollectiveMatchRoot.
 *
 *  @date 25.04.2012
 *  @author Fabian Haensel, Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#ifndef DCOLLECTIVEMATCHROOT_H
#define DCOLLECTIVEMATCHROOT_H

#include "ModuleBase.h"

#include "I_DCollectiveMatchRoot.h"
#include "DCollectiveMatch.h"

using namespace gti;

namespace must
{
    /**
     * Instantiation of distributed collective matching and verification.
     * This is the reduction part that does the final matching on the
     * TBON root.
     */
    class DCollectiveMatchRoot : public DCollectiveMatch<DCollectiveMatchRoot, I_DCollectiveMatchRoot>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DCollectiveMatchRoot (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~DCollectiveMatchRoot (void);

    };
} /*namespace MUST*/

#endif /*DCOLLECTIVEMATCHROOT_H*/
