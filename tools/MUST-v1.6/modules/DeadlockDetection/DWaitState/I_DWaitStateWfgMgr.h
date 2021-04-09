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
 * @file I_DWaitStateWfgMgr.h
 *       @see I_DWaitStateWfgMgr.
 *
 *  @date 08.03.2013
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_DWAITSTATEWFGMGR_H
#define I_DWAITSTATEWFGMGR_H

/**
 * Gathers WFG information from all DWaitState modules if a timeout
 * occurs. It uses this information to detect deadlocks and to report them.
 *
 * This is a clear scalability bottleneck, to advance scalability, a distributed
 * implementation will ultimately become necessary.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 * - CreateMessage
 * - DCollectiveMatchReduction
 *
 */
class I_DWaitStateWfgMgr : public gti::I_Module
{
public:

    /**
     * Notification of a rank without an active blocking op.
     *
     * @param worldRank originating MPI_COMM_WORLD rank
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN  waitForInfoEmpty (
            int worldRank
    ) = 0;

    /**
     * Notification of a ranks or of a ranks sub node wait for information.
     *
     * @param worldRank originating MPI_COMM_WORLD rank
     * @param pId to name the node
     * @param lid to name the node
     * @param subId sub id (if given world rank used provideWaitForInfosMixed), or  -1 if this is for the rank directly
     * @param count number of wait-for dependencies
     * @param type must::ArcType (AND or OR)
     * @param toRanks count sized list of MPI_COMM_WORLD target ranks
     * @param labelPIds optional pId+lId pair for each wait-for, size of count; use 0+0 for NO pair
     * @param labelLIds as for labelPIds, second half
     * @param labelSize string length of all concatenated labels
     * @param labels all labels concatenated and separated with a '\n'
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN  waitForInfoSingle (
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
    ) = 0;

    /**
     * Notification of a ranks WFG information, the rank uses
     * mixed AND and OR semantic, where it specifies its number
     * of sub nodes. For each sub node it will create one
     * waitForInfoSingle event.
     *
     * @param worldRank originating MPI_COMM_WORLD rank
     * @param pId to name the node
     * @param lid to name the node
     * @param numSubs number of sub-ids needed for this rank
     * @param type must::ArcType (AND or OR)
     * @param labelPIds optional pId+lId pair for each wait-for, size of numSubs; use 0+0 for NO pair
     * @param labelLIds as for labelPIds, second half
     * @param labelSize string length of all concatenated labels
     * @param labels all labels concatenated and separated with a '\n'
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN waitForInfoMixed (
            int worldRank,
            MustParallelId pId,
            MustLocationId lId,
            int numSubs,
            int type,
            MustParallelId *labelPIds,
            MustLocationId *labelLIds,
            int labelsSize,
            char *labels
    ) = 0;

    /**
     * Notification of a rank being in a collective. Since the DWaitState
     * module does not knows which other ranks are in the collective,
     * it describes its collective call instead.
     *
     * @param worldRank originating MPI_COMM_WORLD rank
     * @param pId to name the node
     * @param lid to name the node
     * @param collType to differentiate different comms on the same communicator
     * Rest is a copy of our internal communicator description, see DWaitStateCollReduction for infos on this.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN waitForInfoColl (
            int worldRank,
            MustParallelId pId,
            MustLocationId lId,
            int collType,
            int isIntercomm,
            unsigned long long contextId,
            int localGroupSize,
            int remoteGroupSize
        ) = 0;

    /**
     * Notification of a rank being in a collective. Since the DWaitState
     * module does not knows which other ranks are in the collective,
     * it describes its collective call instead.
     *
     * @param worldRank originating MPI_COMM_WORLD rank
     * @param pId to name the node
     * @param lid to name the node
     * @param waveNumInColl number of the wave within the communicator to discern distinct ongoing nbcs on the same comm.
     * Rest is a copy of our internal communicator description, see DWaitStateCollReduction for infos on this.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN waitForBackgroundNbc (
            int worldRank,
            MustParallelId pId,
            MustLocationId lId,
            int waveNumInColl,
            int isIntercomm,
            unsigned long long contextId,
            int localGroupSize,
            int remoteGroupSize
        ) = 0;

    /**
     * Notification of a rank being in a collective. Since the DWaitState
     * module does not knows which other ranks are in the collective,
     * it describes its collective call instead.
     *
     * @param worldRank originating MPI_COMM_WORLD rank
     * @param pId to name the node
     * @param lid to name the node
     * @param subId sub id (if given world rank used provideWaitForInfosMixed), or  -1 if this is for the rank directly
     * @param waveNumInColl number of the wave within the communicator to discern distinct ongoing nbcs on the same comm.
     * Rest is a copy of our internal communicator description, see DWaitStateCollReduction for infos on this.
     * @return @see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN waitForInfoNbcColl (
            int worldRank,
            MustParallelId pId,
            MustLocationId lId,
            int subId,
            int waveNumInColl,
            int isIntercomm,
            unsigned long long contextId,
            int localGroupSize,
            int remoteGroupSize
        ) = 0;

    /**
     * Notification of some heads to acknowledge that they arrived at a
     * consistent state.
     * @param numHeads to acknowledge.
     */
    virtual gti::GTI_ANALYSIS_RETURN acknowledgeConsistentState (
            int numHeads) = 0;

    /**
     * Used to gather all WFG information based on a timeout.
     */
    virtual void timeout (void) = 0;

    /**
     * Triggered if a collective requests an acknowledge.
     * Used to reset the time point of the last seen activity.
     */
    virtual gti::GTI_ANALYSIS_RETURN collActivityNotify (void) = 0;

};/*class I_DWaitStateWfgMgr*/

#endif /*I_DWAITSTATEWFGMGR_H*/
