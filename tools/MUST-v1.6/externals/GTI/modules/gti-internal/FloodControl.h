/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file FloodControl.h
 *       @see MUST::FloodControl.
 *
 *  @date 26.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "ModuleBase.h"

#include "I_FloodControl.h"

#include <vector>

#ifndef FLOODCONTROL_H
#define FLOODCONTROL_H

#define DISABLE_THRESHOLD 1000000
#define ENABLE_HISTERESE 100000

namespace gti
{
    class PriorityListEntry; /**< Forward declaration.*/

    /**
     * Information on state of a channel.
     */
    class StateInfo
    {
    public:
        StateInfo (void);
        virtual ~StateInfo (void) {}

        unsigned int numBad;
        unsigned int numFailedTests;
        long int queueSize;
        bool enabled;
        std::list<PriorityListEntry>::iterator priorityPos;
    };

    /**
     * An entry in the list we use to prioritize the tests that the placement
     * driver fires;
     */
    class PriorityListEntry
    {
    public:
        PriorityListEntry (void);
        virtual ~PriorityListEntry (void) {}

        StateInfo *state;
        int channel;
        GTI_STRATEGY_TYPE direction;
    };

    /**
     * Implementation of I_FloodControl.
     */
    class FloodControl : public ModuleBase<FloodControl, I_FloodControl>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        FloodControl (const char* instanceName);

        /**
         * Destructor.
         */
        virtual ~FloodControl (void);

        /**
         * @see I_FloodControl::init
         */
        GTI_ANALYSIS_RETURN init (
                unsigned int numDownChannels,
                bool hasIntra,
                unsigned int numIntraChannels,
                bool hasUp);

        /**
         * @see I_FloodControl::setCurrentRecordInfo
         */
        GTI_ANALYSIS_RETURN setCurrentRecordInfo (GTI_STRATEGY_TYPE direction, unsigned int channel);

        /**
         * @see I_FloodControl::getCurrentRecordInfo
         */
        GTI_ANALYSIS_RETURN getCurrentRecordInfo (GTI_STRATEGY_TYPE *outDirection, unsigned int *outChannel);

        /**
         * @see I_FloodControl::markCurrentRecordBad
         */
        GTI_ANALYSIS_RETURN markCurrentRecordBad (void);

        /**
         * @see I_FloodControl::markCurrentRecordBad
         */
        GTI_ANALYSIS_RETURN getCurrentTestDecision (gti::GTI_STRATEGY_TYPE *outDirection, unsigned int *outChannel);

        /**
         * @see I_FloodControl::markCurrentRecordBad
         */
        GTI_ANALYSIS_RETURN nextTestDecision (void);

        /**
         * @see I_FloodControl::markCurrentRecordBad
         */
        GTI_ANALYSIS_RETURN rewindDecision (void);

        /**
         * @see I_FloodControl::modifyQueueSize
         */
        GTI_ANALYSIS_RETURN modifyQueueSize (GTI_STRATEGY_TYPE direction, unsigned int channel, int diff);

        /**
         * @see I_FloodControl::getMaxBadness
         */
        uint64_t getMaxBadness (void);

    protected:

        std::vector<StateInfo> myDownStates;
        bool myHasIntra;
        StateInfo myIntraState;
        bool myHasUp;
        StateInfo myUpState;

        GTI_STRATEGY_TYPE myCurDirection;
        unsigned int myCurChannel;
        bool myCurWasReported; /**< Used to avoid cases where a record is marked multiple times as bad.*/

        GTI_STRATEGY_TYPE myMaxDirection;
        unsigned int myMaxChannel;
        unsigned int myMaxBad;

        bool myUsePriority;
        std::list<PriorityListEntry> myPriority;
        std::list<PriorityListEntry>::iterator myNextDecision;

        /**
         * Updates the ranking of pos after its badness or #tries was updated.
         * @param pos to update.
         */
        void updatePriorityList (std::list<PriorityListEntry>::iterator pos);

    }; /*FloodControl*/
} /*namespace gti*/

#endif /*FLOODCONTROL_H*/
