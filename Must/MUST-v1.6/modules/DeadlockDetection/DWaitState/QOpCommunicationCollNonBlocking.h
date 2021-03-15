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
 * @file QOpCommunicationColl.h
 *       @see must::QOpCommunicationColl.
 *
 *  @date 28.07.2015
 *  @author Tobias Hilbrich
 */

#ifndef QOPCOMMUNICATIONCOLLNONBLOCKING_H
#define QOPCOMMUNICATIONCOLLNONBLOCKING_H

#include "QOpCommunicationColl.h"

using namespace gti;

namespace must
{
    /**
     * A non blocking collective communication operation.
     */
    class QOpCommunicationCollNonBlocking : public QOpCommunicationColl
    {
    public:

        /**
         * Constructor for a non blocking collective communication operation.
         * @see QOpCommunicationColl::QOpCommunicationColl
         * @param request of the collective.
         */
        QOpCommunicationCollNonBlocking (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                I_CommPersistent *comm,
                MustCollCommType collType,
                MustLTimeStamp waveNumberInComm,
                MustRequestType request
        );

        /**
         * @see QOpCommunication::hasRequest
         * Returns true.
         */
        virtual bool hasRequest (void);

        /**
         * @see QOpCommunication::getRequest
         * Returns the request.
         */
        virtual MustRequestType getRequest (void);

        /**
         * @see QOp::printVariablesAsLabelString
         */
        virtual std::string printVariablesAsLabelString (void);

        /**
         * @see QOp::blocks
         */
        virtual bool blocks (void);

    protected:
        MustRequestType myRequest;

        /**
         * Destructor.
         */
        virtual ~QOpCommunicationCollNonBlocking (void);
    };

} /*namespace must*/

#endif /*QOPCOMMUNICATIONCOLLNONBLOCKING_H*/
