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
 * @file I_DCollectiveListener.h
 *       @see I_DCollectiveListener.
 *
 *  @date 02.03.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#include "I_Comm.h"

#include <list>

#ifndef I_DCOLLECTIVELISTENER_H
#define I_DCOLLECTIVELISTENER_H

namespace must
{
    /**
     * Interface for listener on collective events.
     */
    class I_DCollectiveListener
    {
    public:

        /**
         * Virtual destructor to avoid inheritance destruction hassles.
         */
        virtual ~I_DCollectiveListener (void) {}

        /**
         * Notifies the listener of a new Collective operation at DP2PCollectiveMatchReduction.
         *
         * @param pId parallel Id of the call site.
         * @param lId location Id of the call site.
         * @see QOpCommunicationColl::QOpCommunicationColl
         * @return the logical timestamp that was given to this op.
         */
        virtual MustLTimeStamp newCollectiveOp (
                MustParallelId pId,
                MustLocationId lId,
                I_CommPersistent *comm,
                MustCollCommType collType,
                MustLTimeStamp waveNumberInComm,
                bool hasRequest,
                MustRequestType request
                ) = 0;

        /**
         * Notifies the listener of a complete collective wave
         * where the list of pIds and timestamps includes all member process of
         * the collective.
         *
         * @param ops pId, lTimeStamp pair for all members in the collective.
         */
        virtual void notifyCollectiveLocalComplete (
                std::list<std::pair<MustParallelId, MustLTimeStamp> > &ops
                ) = 0;

    };/*class I_DCollectiveListener*/
}

#endif /*I_DCOLLECTIVELISTENER_H*/
