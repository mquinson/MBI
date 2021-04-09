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
 * @file DWaitStateWfgMgr.h
 *       @see DWaitStateWfgMgr.
 *
 *  @date 08.03.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"
#include "I_CreateMessage.h"
#include "I_DCollectiveMatchReduction.h"
#include "I_CollCommListener.h"

#include "I_DWaitStateWfgMgr.h"
#include "DistributedDeadlockApi.h"

#include <string>
#include <map>
#include <list>
#include <sys/time.h>

#ifndef DWAITSTATECOLLMGR_H
#define DWAITSTATECOLLMGR_H

using namespace gti;

namespace must
{
    /**
     * Implementation of I_DWaitStateWfgMgr.
     * @see I_DWaitStateWfgMgr
     */
    class DWaitStateWfgMgr
        : public gti::ModuleBase<DWaitStateWfgMgr, I_DWaitStateWfgMgr>, public I_CollCommListener
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        DWaitStateWfgMgr (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~DWaitStateWfgMgr (void);

        /**
         * @see I_DWaitStateWfgMgr::waitForInfoEmpty
         */
        GTI_ANALYSIS_RETURN  waitForInfoEmpty (
                    int worldRank
            );

        /**
         * @see I_DWaitStateWfgMgr::waitForInfoSingle
         */
        GTI_ANALYSIS_RETURN  waitForInfoSingle (
                int worldRank,
                MustParallelId pId,
                MustLocationId lId,
                int subId,
                int count,
                int type,
                int* toRanks,
                MustParallelId *labelPIds,
                MustLocationId *labelLIds,
                int labelsSize,
                char *labels
        );

        /**
         * @see I_DWaitStateWfgMgr::waitForInfoMixed
         */
        GTI_ANALYSIS_RETURN waitForInfoMixed (
                int worldRank,
                MustParallelId pId,
                MustLocationId lId,
                int numSubs,
                int type,
                MustParallelId *labelPIds,
                MustLocationId *labelLIds,
                int labelsSize,
                char *labels
        );

        /**
         * @see I_DWaitStateWfgMgr::waitForInfoColl
         */
        GTI_ANALYSIS_RETURN waitForInfoColl (
                int worldRank,
                MustParallelId pId,
                MustLocationId lId,
                int collType,
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize
        );

        /**
         * @see I_DWaitStateWfgMgr::waitForBackgroundNbc
         */
        GTI_ANALYSIS_RETURN waitForBackgroundNbc (
                int worldRank,
                MustParallelId pId,
                MustLocationId lId,
                int waveNumInColl,
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize
        );

        /**
         * @see I_DWaitStateWfgMgr::waitForInfoNbcColl
         */
        GTI_ANALYSIS_RETURN waitForInfoNbcColl (
                int worldRank,
                MustParallelId pId,
                MustLocationId lId,
                int subId,
                int waveNumInColl,
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize
        );

        /**
         * @see I_DWaitStateWfgMgr::acknowledgeConsistentState
         */
        GTI_ANALYSIS_RETURN acknowledgeConsistentState (
                int numHeads);

        /**
         * @see I_DWaitStateWfgMgr::timeout
         */
        void timeout (void);

        /**
         * @see I_DWaitStateWfgMgr::collActivityNotify
         */
        GTI_ANALYSIS_RETURN collActivityNotify (void);

        /**
         * @see I_CollCommListener::newCommInColl
         */
        void newCommInColl (
                MustParallelId pId,
                I_CommPersistent* comm);

    protected:
        unsigned long long myLastActivity;
        requestWaitForInfosP myFRequestInfos;
        requestConsistentStateP myFRequestConsistentState;
        I_ParallelIdAnalysis* myPIdMod;
        I_LocationAnalysis* myLIdMod;
        I_CreateMessage *myLogger;
        I_DCollectiveMatchReduction *myDColl;

        class nodeInfo
        {
        public:
            bool isSubNode;
            int nodeId; /*worldRank (if not subNode), otherwise subId*worldSize+worldRank*/
            int worldRank;
            MustParallelId pId;
            MustLocationId lId;
            ArcType type;
            std::list<int> toNodes; /*node id, not rank*/
            std::list<std::string> toLabels;
            bool isDeadlocked;

            nodeInfo (void)
              : isSubNode (false),
                nodeId (0),
                worldRank (0),
                pId (0),
                lId (0),
                type (ARC_AND),
                toNodes(),
                toLabels(),
                isDeadlocked(false) {}
        };

        class commInfo
        {
        public:
            I_CommPersistent *comm;
            /**
             * Participation information for blocking collectives.
             * Maps collective type to a map that maps world rank to pId,lId pairs.
             */
            std::map<MustCollCommType, std::map<int, std::pair<MustParallelId, MustLocationId> > > colls;

            /**
             * Participation information (called background) for non-blocking collectives.
             * Maps waveNumber to a map that maps world rank to pId,lId pairs.
             */
            std::map<int, std::map<int, std::pair<MustParallelId, MustLocationId> > > activeNbcs;

            /**
             * Operations that wait within this collective for other ranks.
             * Maps nodeId (worldRank (if not subNode), otherwise subId*worldSize+worldRank)
             *      to a map that maps wave number to the pId,lId pair of this sub-operation.
             */
            std::map<int, std::map<int, std::pair<MustParallelId, MustLocationId> > > waitingNbcs;

            commInfo (void) : comm(NULL), colls(), activeNbcs() {}
        };

        std::list<commInfo> myCommInfos;
        std::map<int, nodeInfo> myNodeInfos; //maps node id (not world rank!) to info
        int myWorldSize;
        int myNumReplies;
        int myExpectedReplies;

        int myNumConsistentReplies;

        bool myWfgRequestActive;

        unsigned long long myTSyncStart, myTSyncEnd, myTWfgInfoArrived, myTPrepFin, myTWfgCheckFin, myTOutputFin, myTDotFin;

        /**
         * Takes all information that arrived and compiles it into
         * a wfg that we check for deadlock and report to the user
         * if we find a deadlock.
         */
        void compileCheckReport (void);

        /**
         * Does the reporting for a deadlock with the given nodes.
         * @param nodes that cause deadlock.
         */
        void reportDeadlock (std::list<int> nodes);

        /**
         * Looks up the commInfo in myCommInfos that matches the given communicator description.
         * Arguments are a copy of our internal communicator description, see DWaitStateCollReduction for infos on this.
         */
        DWaitStateWfgMgr::commInfo* getCommInfo (
                int isIntercomm,
                unsigned long long contextId,
                int localGroupSize,
                int remoteGroupSize);

        /**
         * Provides a usec time
         */
        inline unsigned long long getUsecTime (void)
        {
            struct timeval t;
            gettimeofday(&t, NULL);
            return t.tv_sec * 1000000 + t.tv_usec;
        }
    };
} /*namespace MUST*/

#endif /*DWAITSTATEWFGMGR_H*/
