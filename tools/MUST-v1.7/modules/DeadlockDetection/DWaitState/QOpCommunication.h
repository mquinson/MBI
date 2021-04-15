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
 * @file QOpCommunication.h
 *       @see must::QOpCommunication.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#ifndef QOPCOMMUNICATION_H
#define QOPCOMMUNICATION_H

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"
#include "I_CommTrack.h"

#include "QOp.h"

using namespace gti;

namespace must
{
    /**
     * Any communication operation.
     */
    class QOpCommunication : public QOp
    {
    public:

        /**
         * Constructor for any communication operation.
         * @see QOp::QOp
         * @param comm communicator that will be managed by this
         *              operation. I.e. it will free this communicator
         *              in its destructor.
         */
        QOpCommunication (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                I_CommPersistent *comm
        );

        /**
         * Returns communicator used by this communication operation.
         * @return comm.
         */
        I_Comm* getComm (void);

        /**
         * @see QOp::printVariablesAsLabelString
         */
        virtual std::string printVariablesAsLabelString (void);

        /**
         * @see QOp::getUsedComms
         */
        virtual std::list<I_Comm*> getUsedComms (void);

        /**
         * Returns true if this is a non-blocking operation.
         * Default implementation in this class return false.
         * @return true iff non-blocking.
         */
        virtual bool hasRequest (void);

        /**
         * If this is a non-blocking operation, returns
         * request associated with this communication.
         * @return request if non-blocking.
         */
        virtual MustRequestType getRequest (void);

        /**
         * Returns true if this ops matching op is active.
         * Note that this does not checks whether this op
         * itself is active.
         *
         * If no matching op is known, returns false
         *
         * @return true iff matching op is active.
         */
        virtual bool isMatchedWithActiveOps (void) = 0;

        /**
         * Forwards this operations wait-for information.
         * Does no blocking checking, i.e., the caller must
         * make certain that this is reasonable.
         * Also only forwards this very information, not any associated
         * sub-operations that we may be linked with (e.g., a sendrecv).
         *
         * @param subIdToUse in the forward call.
         * @param commLabels input labels to use for communicator.
         */
        virtual void forwardThisOpsWaitForInformation (
                int subIdToUse,
                std::map<I_Comm*, std::string> &commLabels) = 0;


    protected:
        I_CommPersistent *myComm;

        /**
         * Destructor.
         */
        virtual ~QOpCommunication (void);
    };

} /*namespace must*/

#endif /*QOPCOMMUNICATION_H*/
