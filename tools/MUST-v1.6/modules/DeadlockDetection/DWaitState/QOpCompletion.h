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
 * @file QOpCommpletion.h
 *       @see must::QOpCommpletion.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"
#include "QOpCommunication.h"

#include "QOp.h"

#include <vector>

#ifndef QOPCOMMPLETION_H
#define QOPCOMMPLETION_H

using namespace gti;

namespace must
{
    /**
     * Helper for information associated with a request.
     */
    class RequestInfo
    {
    public:
        MustRequestType request;
        QOpCommunication* nonBlockingOp;
        bool completed;

        /**
         * Initialized request info, the memory of the p2pOp
         * is managed by this object.
         * @param request value.
         * @param associated p2pOp.
         */
        RequestInfo (
                MustRequestType request,
                QOpCommunication* nonBlockingOp);
        RequestInfo (void);
        ~RequestInfo (void);
    };

    /**
     * A completion operation (MPI_Wait...).
     */
    class QOpCompletion : public QOp
    {
    public:

        /**
         * Constructor for a MPI_Wait.
         * @see QOp::QOp
         * @param request in the MPI_Wait call (only valid, non MPI_PROC_NULL, and active requests; Due to preconditioner).
         */
        QOpCompletion (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                MustRequestType request
        );

        /**
         * Constructor for a MPI_Wait.
         * @see QOp::QOp
         * @param count of requests in array.
         * @param requests array (only valid, non MPI_PROC_NULL, and active requests; Due to preconditioner).
         * @param waitsForAll true iff this is MPI_Waitall, otherwise its either MPI_Waitany or MPI_Waitsome.
         * @param hadProcNullReqs true if the original array (before preconditioner) had a MPI_PROC_NULL request
         *               (Necessary to understand that MPI_Waitany, MPI_Waitsome can trivially complete).
         */
        QOpCompletion (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                int count,
                MustRequestType *requests,
                bool waitsForAll,
                bool hadProcNullReqs
        );

        /**
         * Used QOp::printAsDot, but adds arcs to the nodes of the non-blocking P2P calls used in here.
         * @see QOp::printAsDot
         */
        virtual std::string printAsDot (std::ofstream &out, std::string nodePrefix, std::string color);

        /**
         * @see QOp::printVariablesAsLabelString
         */
        virtual std::string printVariablesAsLabelString (void);

        /**
         * @see QOp::notifyActive
         */
        virtual void notifyActive (void);

        /**
         * @see QOp::notifyActive
         */
        virtual bool blocks (void);

        /**
         * @see QOp::needsToBeInTrace
         */
        virtual bool needsToBeInTrace (void);

        /**
         * @see QOp::forwardWaitForInformation
         */
        virtual void forwardWaitForInformation (
                std::map<I_Comm*, std::string> &commLabels);

        /**
         * @see QOp::getUsedComms
         */
        virtual std::list<I_Comm*> getUsedComms (void);

        /**
         * @see QOp::getPingPongNodes
         */
        virtual std::set<int> getPingPongNodes (void);

    protected:
        RequestInfo myRequest;
        std::vector<RequestInfo> myRequests;

        bool myWaitsForAll;
        int myNumCompleted;
        int myMatchIndex; /**< Index of first completed request.*/

        /**
         * Destructor.
         */
        virtual ~QOpCompletion (void);
    };

} /*namespace must*/

#endif /*QOPCOMMPLETION_H*/
