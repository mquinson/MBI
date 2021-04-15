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
 * @file OverlapChecks.h
 *       @see must::OverlapChecks.
 *
 *  @date 27.05.2011
 *  @author Joachim Protze
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"
#include "I_DatatypeTrack.h"
#include "I_RequestTrack.h"
#include "I_LocationAnalysis.h"

#include "I_OverlapChecks.h"
#include "StridedBlock.h"

#include <map>


#ifndef OVERLAPPCHECKS_H
#define OVERLAPPCHECKS_H

using namespace gti;

namespace must
{
    /**
    * Datatype used for Memoryintervals.
    */
    template <class T>
    class mustPidMap : public std::map<int, T > {};

    template <class T>
    class mustPidDatatypeMap : public std::map<int, std::map<MustDatatypeType, T > > {};

    template <class T>
    class mustPidRequestMap : public std::map<int, std::map<MustRequestType, T > > {};

    enum overlapState 
    {
        IS_OVERLAPPED = 0,
        IS_NOT_OVERLAPPED,
        UNKNOWN_OVERLAP
    };

    /**
     * OverlapChecks for correctness checks interface implementation.
     */
    class OverlapChecks : public gti::ModuleBase<OverlapChecks, I_OverlapChecks>
    {

        public:
            double profil[10];
            /**
            * Constructor.
            * @param instanceName name of this module instance.
            */
            OverlapChecks (const char* instanceName);

            /**
             * Destructor.
             */
            virtual ~OverlapChecks (void);


        public:

            /**
             * @see I_OverlapChecks::isTypeOverlappedN.
             */
            GTI_ANALYSIS_RETURN isTypeOverlappedN (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype,
                    int count);

            /**
             * @see I_OverlapChecks::warnIfTypeOverlapped.
             */
            GTI_ANALYSIS_RETURN warnIfTypeOverlapped (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);
            
            /**
             * @see I_OverlapChecks::warnIfTypeOverlappedN.
             */
            GTI_ANALYSIS_RETURN warnIfTypeOverlappedN (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype,
                    int count);
            
            /**
             * @see I_OverlapChecks::isSendOverlappedN.
             */
            GTI_ANALYSIS_RETURN isSendOverlappedN (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    int count);

            /**
             * @see I_OverlapChecks::isRecvOverlappedN.
             */
            GTI_ANALYSIS_RETURN isRecvOverlappedN (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    int count);

            /**
             * @see I_OverlapChecks::isSendRecvOverlapped.
             */
            GTI_ANALYSIS_RETURN isSendRecvOverlapped (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustAddressType sendbuf,
                    int sendcount,
                    MustDatatypeType sendtype,
                    MustAddressType recvbuf,
                    int recvcount,
                    MustDatatypeType recvtype
                    );

            /**
             * @see I_OverlapChecks::isSendRecvOverlappedN.
             */
            GTI_ANALYSIS_RETURN isSendRecvOverlappedN (
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
                    );

            /**
             * @see I_OverlapChecks::isTypeOverlapped.
             */
            GTI_ANALYSIS_RETURN isTypeOverlapped (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);

        protected:

            bool checkTypeOverlapped (
                    I_Datatype * datatype,
                    int extent,
                    int count);

            bool isOverlappedN(
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    int count);

            bool setSelfOverlapCache(
                    I_Datatype * datatype,
                    int count,
                    bool overlaps);


            MustMemIntervalListType calcIntervalList(
                    I_Datatype * datatype,
                    MustAddressType buffer,
                    int count,
                    MustRequestType request,
                    bool isSend,
                    bool* hasoverlap = NULL);

            GTI_ANALYSIS_RETURN makeBlocksActive(
                    MustParallelId pId,
                    MustLocationId lId,
                    MustMemIntervalListType & preparedList,
                    MustRequestType request
                    );

            GTI_ANALYSIS_RETURN makeBlocksInActive(
                    int rank,
                    MustRequestType request
                    );

            GTI_ANALYSIS_RETURN makeBlocksInActive(
                    MustParallelId pId,
                    MustRequestType request
                    );

            GTI_ANALYSIS_RETURN checkOverlapsRequests (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustMemIntervalListType & iList,
                    bool isSend,
                    void (must::OverlapChecks::*outputFunction)(MustParallelId,MustLocationId,MustRequestType));

            GTI_ANALYSIS_RETURN checkOverlapsRequests (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustMemIntervalListType & iList,
                    bool isSend,
                    const char* errstring, enum MustMessageIdNames errcode);
            
            void outputSendOverlapsRequests(
                    MustParallelId pId,
                    MustLocationId lId,
                    MustRequestType request);

            void outputRecvOverlapsRequests(
                    MustParallelId pId,
                    MustLocationId lId,
                    MustRequestType request);

            void outputStartPRequest(
                    MustParallelId pId,
                    MustLocationId lId,
                    MustRequestType request);

            #ifdef DOT
            void generateOverlapHtml (std::string dotFile, std::string htmlFile, std::string imageFile);
            #endif

        public:

            /**
             * @see I_OverlapChecks::sendOverlapcheckCounts.
             */
            GTI_ANALYSIS_RETURN sendOverlapcheckCounts (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustAddressType buffer,
                    const int displs[],
                    const int counts[],
                    MustDatatypeType datatype,
                    int commsize,
 	            int hasRequest,
                    MustRequestType request
                    );

            /**
             * @see I_OverlapChecks::recvOverlapcheckCounts.
             */
            GTI_ANALYSIS_RETURN recvOverlapcheckCounts (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustAddressType buffer,
                    const int displs[],
                    const int counts[],
                    MustDatatypeType datatype,
                    int commsize,
 	            int hasRequest,
                    MustRequestType request
                    );

            /**
             * @see I_OverlapChecks::sendOverlapcheckTypes.
             */
            GTI_ANALYSIS_RETURN sendOverlapcheckTypes (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustAddressType buffer,
                    const int displs[],
                    const int counts[],
                    const MustDatatypeType datatypes[],
                    int commsize,
 	            int hasRequest,
                    MustRequestType request
                    );

            /**
             * @see I_OverlapChecks::recvOverlapcheckTypes.
             */
            GTI_ANALYSIS_RETURN recvOverlapcheckTypes (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustAddressType buffer,
                    const int displs[],
                    const int counts[],
                    const MustDatatypeType datatypes[],
                    int commsize,
 	            int hasRequest,
                    MustRequestType request
                    );

            /**
             * @see I_OverlapChecks::overlapsRequests.
             */
            GTI_ANALYSIS_RETURN overlapsRequests (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
                    char isSend);

            /**
             * @see I_OverlapChecks::sendOverlapsRequests.
             */
            GTI_ANALYSIS_RETURN sendOverlapsRequests (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count);

            /**
             * @see I_OverlapChecks::recvOverlapsRequests.
             */
            GTI_ANALYSIS_RETURN recvOverlapsRequests (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count);

            /**
             * @see I_OverlapChecks::isendOverlapsRequests.
             */
            GTI_ANALYSIS_RETURN isendOverlapsRequests (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
 	            int hasRequest,
                    MustRequestType request
                    );

            /**
             * @see I_OverlapChecks::irecvOverlapsRequests.
             */
            GTI_ANALYSIS_RETURN irecvOverlapsRequests (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
 	            int hasRequest,
                    MustRequestType request
                    );

            /**
             * @see I_OverlapChecks::announceRequest.
             */
            GTI_ANALYSIS_RETURN announceRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
                    char isSend,
                    MustRequestType request);

		/* overloaded version for internal use */
            GTI_ANALYSIS_RETURN announceRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustMemIntervalListType iList,
                    MustRequestType request);

            /**
             * @see I_OverlapChecks::announceSendRequest.
             */
            GTI_ANALYSIS_RETURN announceSendRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
                    MustRequestType request);

            /**
             * @see I_OverlapChecks::announceRecvRequest.
             */
            GTI_ANALYSIS_RETURN announceRecvRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
                    MustRequestType request);

            /**
             * @see I_OverlapChecks::announcePRequest.
             */
            GTI_ANALYSIS_RETURN announcePRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
                    char isSend,
                    MustRequestType request);

            /**
             * @see I_OverlapChecks::announcePSendRequest.
             */
            GTI_ANALYSIS_RETURN announcePSendRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
                    MustRequestType request);

            /**
             * @see I_OverlapChecks::announcePRecvRequest.
             */
            GTI_ANALYSIS_RETURN announcePRecvRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustDatatypeType datatype,
                    MustAddressType buffer,
                    int count,
                    MustRequestType request);

            /**
             * @see I_OverlapChecks::startPRequest.
             */
            GTI_ANALYSIS_RETURN startPRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustRequestType request);

            /**
             * @see I_OverlapChecks::startPRequestArray.
             */
            GTI_ANALYSIS_RETURN startPRequestArray (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustRequestType* request,
                    int count);

            /**
             * @see I_OverlapChecks::finishRequest.
             */
            GTI_ANALYSIS_RETURN finishRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustRequestType request);

            /**
             * @see I_OverlapChecks::finishRequests.
             */
            GTI_ANALYSIS_RETURN finishRequests (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustRequestType* request,
                    int count);

            /**
             * @see I_OverlapChecks::freeRequest.
             */
            GTI_ANALYSIS_RETURN freeRequest (
                    MustParallelId pId,
                    MustLocationId lId,
                    MustRequestType request);

        protected:
            I_ParallelIdAnalysis* myPIdMod;
            I_CreateMessage* myLogger;
            I_ArgumentAnalysis* myArgMod;
            I_DatatypeTrack* myDatMod;
            I_RequestTrack* myReqMod;
            I_LocationAnalysis *myLIdMod;
            /* simplify your life */
            int pId2Rank(
                    MustParallelId pId);
            std::string graphFileName(
                    MustParallelId pId);
            MustMemIntervalListType lastIntervallist;
            bool lastOverlap;
            I_Datatype * lastInfo;
            MustAddressType lastBuffer;
            int lastCount;
            bool doDotOutput;

        protected:
            /* list of memory blocks, that are used in an active communication */
            mustPidMap< MustMemIntervalListType > activeBlocks;
            /* list of memory blocks, that are prepared for persistent requests */
            mustPidRequestMap< MustMemIntervalListType > preparedBlocklists;
            /* list of pointers to activeBlocks for fast removing a finished request from this list */
            mustPidRequestMap< std::list< MustMemIntervalListType::iterator > > activeRequestsBlocklists;
            /* list of location/process information for the requests */
            mustPidRequestMap< std::pair< MustParallelId, MustLocationId > > requestLocation;

    }; /*class OverlapChecks*/
} /*namespace must*/

#endif /*OVERLAPPCHECKS_H*/
