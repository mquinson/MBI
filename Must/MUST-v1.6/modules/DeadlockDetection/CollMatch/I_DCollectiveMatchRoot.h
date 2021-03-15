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
 * @file I_DCollectiveMatchRoot.h
 *       @see I_DCollectiveMatchRoot.
 *
 *  @date 25.04.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#ifndef I_COLLECTIVEMATCHROOT_H
#define I_COLLECTIVEMATCHROOT_H

#include "I_Reduction.h"
#include "I_DCollectiveMatch.h"

using namespace must;

/**
 * Interface for distributed collective matching.
 * Version for reduction running on the TBON
 * root.
 */
class I_DCollectiveMatchRoot : public I_DCollectiveMatch, public gti::I_Reduction
{
public:
    virtual ~I_DCollectiveMatchRoot() {};

    /**
     * We listen to timeouts to trigger flush requests if nothing happens.
     */
    virtual void timeout (void) = 0;
};

#endif /*I_COLLECTIVEMATCHROOT_H*/
