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
 * @file I_P2PMatchListener.h
 *       @see I_P2PMatchListener.
 *
 *  @date 10.08.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "BaseIds.h"
#include "MustEnums.h"
#include "MustTypes.h"

#ifndef I_P2PMATCHLISTENER_H
#define I_P2PMATCHLISTENER_H

namespace must
{
    /**
     * Interface to be implemented by listeners on P2P matches.
     */
    class I_P2PMatchListener
    {
    public:
        /**
         * Called when a new P2P match occurs.
         */
        virtual void newMatch (
                int sendRankWorld,
                int receiveRankWorld,
                bool sendHasRequest,
                MustRequestType sendRequest,
                bool receiveHasRequest,
                MustRequestType receiveRequest,
                must::MustSendMode sendMode) = 0;

    };/*class I_P2PMatchListener*/
} /*namespace must*/

#endif /*I_P2PMATCHLISTENER_H*/
