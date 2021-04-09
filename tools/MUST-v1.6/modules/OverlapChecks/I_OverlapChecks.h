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
 * @file I_OverlapChecks.h
 *       @see I_OverlapChecks
 *
 *  @date 20.05.2011
 *  @author Joachim Protze
 */

// #include "I_Module.h"
#include "GtiEnums.h"

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"

// #include <list>

#ifndef I_OVERLAPCHECKS_H
#define I_OVERLAPCHECKS_H


/**
 * Interface for checking datatypes for illegal overlaps.
 *
 * Dependencies (in listed order):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 * - DatatypeTrack
 * - RequestTrack
 */
class I_OverlapChecks : public gti::I_Module
{
public:

        /**
         * Checks whether the buffers of a sendrecv-call overlap.
         * @param pId parallel context
         * @param lId location id of context.
         * @param sendbuf buffer for the sendpart.
         * @param sendcount number of repetitions for the sendpart.
         * @param sendtype datatype for the sendpart.
         * @param recvbuf buffer for the recvpart.
         * @param recvcount number of repetitions for the recvpart.
         * @param recvtype datatype for the recvpart.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN isSendRecvOverlapped (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                int sendcount,
                MustDatatypeType sendtype,
                MustAddressType recvbuf,
                int recvcount,
                MustDatatypeType recvtype
                )=0;

        /**
         * Checks whether the buffers of a sendrecv-kind-call overlap.
         * @param pId parallel context
         * @param lId location id of context.
         * @param sendbuf buffer for the sendpart.
         * @param senddispls displacements for the sendpart.
         * @param senddisplslen arraysize of displacements for the sendpart.
         * @param sendcounts numbers of repetitions for the sendpart.
         * @param sendcountslen arraysize of numbers of repetitions for the sendpart.
         * @param sendtypes datatypes for the sendpart.
         * @param sendtypeslen arraysize of datatypes for the sendpart.
         * @param recvbuf buffer for the recvpart.
         * @param recvdispls displacements for the recvpart.
         * @param recvdisplslen arraysize of displacements for the recvpart.
         * @param recvcounts numbers of repetitions for the recvpart.
         * @param recvcountslen arraysize of numbers of repetitions for the recvpart.
         * @param recvtypes datatypes for the recvpart.
         * @param recvtypeslen arraysize of datatypes for the recvpart.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN isSendRecvOverlappedN (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType sendbuf,
                const int *senddispls,
                int senddisplslen,
                const int *sendcounts,
                int sendcountslen,
                const MustDatatypeType *sendtypes,
                int sendtypeslen,
                MustAddressType recvbuf,
                const int *recvdispls,
                int recvdisplslen,
                const int *recvcounts,
                int recvcountslen,
                const MustDatatypeType *recvtypes,
                int recvtypeslen
                )=0;

        /**
         * Checks whether a type overlaps after count repetitions or not.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param count number of repetitions.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN isTypeOverlappedN (
                MustParallelId pId,
                MustLocationId lId,
                int aId,
                MustDatatypeType datatype,
                int count)=0;

        /**
         * Checks whether a type overlaps after count repetitions or not.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param count number of repetitions.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN isRecvOverlappedN (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                int count)=0;

        /**
         * Checks whether a type overlaps after count repetitions or not.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param count number of repetitions.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN isSendOverlappedN (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                int count)=0;

        /**
         * Checks whether a type overlaps with itself.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN isTypeOverlapped (
                MustParallelId pId,
                MustLocationId lId,
                int aId,
                MustDatatypeType datatype)=0;

        /**
         * Checks whether a type overlaps with itself.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN warnIfTypeOverlapped (
                MustParallelId pId,
                MustLocationId lId,
                int aId,
                MustDatatypeType datatype)=0;
                
        /**
         * Checks whether a type overlaps with itself.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param count number of repetitions.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN warnIfTypeOverlappedN (
                MustParallelId pId,
                MustLocationId lId,
                int aId,
                MustDatatypeType datatype,
                int count)=0;

        /**
         * Checks whether a request overlaps memory regions spanned by open requests.
         * @param pId parallel context
         * @param lId location id of context.
         * @param buffer address of transfer buffer.
         * @param displs displacements for each block.
         * @param counts lengths of each block.
         * @param datatype to get references for.
         * @param commsize number of ranks.
         * @param hasRequest specify whether call has a request.
         * @param request for this communication call.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN sendOverlapcheckCounts (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType buffer,
                const int displs[],
                const int counts[],
                MustDatatypeType datatype,
                int commsize,
                int hasRequest,
                MustRequestType request
                )=0;

        /**
         * Checks whether a request overlaps memory regions spanned by open requests.
         * @param pId parallel context
         * @param lId location id of context.
         * @param buffer address of transfer buffer.
         * @param displs displacements for each block.
         * @param counts lengths of each block.
         * @param datatype to get references for.
         * @param commsize number of ranks.
         * @param hasRequest specify whether call has a request.
         * @param request for this communication call.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN recvOverlapcheckCounts (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType buffer,
                const int displs[],
                const int counts[],
                MustDatatypeType datatype,
                int commsize,
                int hasRequest,
                MustRequestType request
                )=0;

        /**
         * Checks whether a request overlaps memory regions spanned by open requests.
         * @param pId parallel context
         * @param lId location id of context.
         * @param buffer address of transfer buffer.
         * @param displs displacements for each block.
         * @param counts lengths of each block.
         * @param datatype to get references for.
         * @param commsize number of ranks.
         * @param hasRequest specify whether call has a request.
         * @param request for this communication call.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN sendOverlapcheckTypes (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType buffer,
                const int displs[],
                const int counts[],
                const MustDatatypeType datatypes[],
                int commsize,
                int hasRequest,
                MustRequestType request
                )=0;

        /**
         * Checks whether a request overlaps memory regions spanned by open requests.
         * @param pId parallel context
         * @param lId location id of context.
         * @param buffer address of transfer buffer.
         * @param displs displacements for each block.
         * @param counts lengths of each block.
         * @param datatype to get references for.
         * @param commsize number of ranks.
         * @param hasRequest specify whether call has a request.
         * @param request for this communication call.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN recvOverlapcheckTypes (
                MustParallelId pId,
                MustLocationId lId,
                MustAddressType buffer,
                const int displs[],
                const int counts[],
                const MustDatatypeType datatypes[],
                int commsize,
                int hasRequest,
                MustRequestType request
                )=0;

        /**
         * Checks whether a request overlaps memory regions spanned by open requests.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param isSend whether Request is send or recv
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN overlapsRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                char isSend)=0;

        /**
         * Convenience function for overlapsRequests where isSend is set to true.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN sendOverlapsRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count)=0;

        /**
         * Convenience function for overlapsRequests where isSend is set to false.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN recvOverlapsRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count)=0;

        /**
         * Convenience function for overlapsRequests where isSend is set to true.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param hasRequest specify whether call has a request.
         * @param request for this communication call.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN isendOverlapsRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                int hasRequest,
                MustRequestType request
                )=0;

        /**
         * Convenience function for overlapsRequests where isSend is set to false.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param hasRequest specify whether call has a request.
         * @param request for this communication call.
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN irecvOverlapsRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                int hasRequest,
                MustRequestType request
                )=0;

        /**
         * Returns whether a persistent request overlaps open requests.
         * @param pId parallel context
         * @param lId location id of context.
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
//         virtual gti::GTI_ANALYSIS_RETURN persistentOverlapsRequests (
//                 MustParallelId pId,
//                 MustLocationId lId,
//                 MustRequestType request)=0;

        /**
         * Announce start of a non-blocking communication.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param isSend whether Request is send or recv
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN announceRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                char isSend,
                MustRequestType request)=0;

        /**
         * Convenience function for announceRequest where isSend is set to true.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN announceSendRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                MustRequestType request)=0;

        /**
         * Convenience function for announceRequest where isSend is set to false.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN announceRecvRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                MustRequestType request)=0;

        /**
         * Announce creation of a persistent request.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param isSend whether Request is send or recv
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN announcePRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                char isSend,
                MustRequestType request)=0;

        /**
         * Convenience function for announcePRequest where isSend is set to true.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN announcePSendRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                MustRequestType request)=0;

        /**
         * Convenience function for announcePRequest where isSend is set to false.
         * @param pId parallel context
         * @param lId location id of context.
         * @param datatype to get references for.
         * @param buffer address of transfer buffer.
         * @param count number of repetitions.
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN announcePRecvRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustDatatypeType datatype,
                MustAddressType buffer,
                int count,
                MustRequestType request)=0;

        /**
         * Start persistent request.
         * @param pId parallel context
         * @param lId location id of context.
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN startPRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request)=0;

        /**
         * Start array of persistent requests.
         * @param pId parallel context
         * @param lId location id of context.
         * @param requests handles of requests
         * @param count number of requests
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN startPRequestArray (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType* requests,
                int count)=0;

        /**
         * Finish request (Notification that a communication associated with the
         * given request was completed).
         * @param pId parallel context
         * @param lId location id of context.
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN finishRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request)=0;

        /**
         * Finish requests (Notification that for each entry in the array the
         * communication associated with the request was completed).
         * @param pId parallel context
         * @param lId location id of context.
         * @param requests handles of requests
         * @param count number of finished requests
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN finishRequests (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType* request,
                int count)=0;

        /**
         * Free prequest (Notification that a (persistent) Request is freed).
         * @param pId parallel context
         * @param lId location id of context.
         * @param request handle of request
         * @return see gti::GTI_ANALYSIS_RETURN.
         */
        virtual gti::GTI_ANALYSIS_RETURN freeRequest (
                MustParallelId pId,
                MustLocationId lId,
                MustRequestType request)=0;


}; /*class I_OverlapChecks*/

#endif /*I_OVERLAPCHECKS_H*/
