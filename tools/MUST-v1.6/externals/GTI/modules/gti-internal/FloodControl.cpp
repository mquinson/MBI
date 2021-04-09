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
 * @file FloodControl.cpp
 *       @see MUST::FloodControl.
 *
 *  @date 26.07.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "GtiMacros.h"
#include "GtiApi.h"

#include "FloodControl.h"

#include <iostream>

using namespace gti;

mGET_INSTANCE_FUNCTION(FloodControl);
mFREE_INSTANCE_FUNCTION(FloodControl);
mPNMPI_REGISTRATIONPOINT_FUNCTION(FloodControl);

//=============================
// StateInfo: Constructor
//=============================
StateInfo::StateInfo ()
 : numBad (0),
   numFailedTests (0),
   queueSize (0),
   enabled (true),
   priorityPos ()
{
    //Nothing to do
}

//=============================
// PriorityListEntry: Constructor
//=============================
PriorityListEntry::PriorityListEntry ()
 : state (NULL),
   channel (0),
   direction (GTI_STRATEGY_DOWN)
{
    //Nothing to do
}

//=============================
// Constructor
//=============================
FloodControl::FloodControl (const char* instanceName)
    : gti::ModuleBase<FloodControl, I_FloodControl> (instanceName),
      myDownStates (),
      myIntraState (),
      myUpState (),
      myCurDirection (GTI_STRATEGY_UP),
      myCurChannel (0),
      myMaxDirection (GTI_STRATEGY_UP),
      myMaxChannel (0),
      myMaxBad (0),
      myHasUp (false),
      myCurWasReported (true),
      myHasIntra (false),
      myNextDecision ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBS 0
/*    if (subModInstances.size() < NUM_SUBS)
    {
            std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
            assert (0);
    }*/
    if (subModInstances.size() > NUM_SUBS)
    {
            for (int i = NUM_SUBS; i < subModInstances.size(); i++)
                destroySubModuleInstance (subModInstances[i]);
    }

    //Initialize module data
    /*nothing to do*/
}

//=============================
// Destructor
//=============================
FloodControl::~FloodControl ()
{
    //Print maxium badness of this TBON node, for measurement purposes
#ifdef MUST_FLOOD_PRINT_MAX
    std::cout << getpid () << " direction=" << myMaxDirection << " myMaxChannel=" << myMaxChannel << " badness=" << myMaxBad << std::endl;
#endif
}

//=============================
// init
//=============================
GTI_ANALYSIS_RETURN FloodControl::init (
                unsigned int numDownChannels,
                bool hasIntra,
                unsigned int numIntraChannels,
                bool hasUp)
{
    std::list<PriorityListEntry>::iterator pos;

    myDownStates.resize(numDownChannels);

    myHasUp = hasUp;
    myHasIntra = hasIntra;

    PriorityListEntry intra,up;
    intra.direction = GTI_STRATEGY_INTRA;
    intra.state = &myIntraState;
    up.direction = GTI_STRATEGY_UP;
    up.state = &myUpState;

    if (myHasUp)
        myPriority.push_back(up);
    if (myHasIntra)
        myPriority.push_back(intra);

    for (int i = 0; i < myDownStates.size(); i++)
    {
        PriorityListEntry entry;
        entry.direction = GTI_STRATEGY_DOWN;
        entry.channel = i;
        entry.state = &(myDownStates[i]);
        myPriority.push_back(entry);
    }

    //Store priority list positions in the state infos
    pos = myPriority.begin();
    if (myHasUp)
    {
        myUpState.priorityPos = pos;
        pos++;
    }
    if (myHasIntra)
    {
        myIntraState.priorityPos = pos;
        pos++;
    }

    for (int i = 0; i < myDownStates.size(); i++, pos++)
        myDownStates[i].priorityPos = pos;

    myNextDecision = myPriority.begin();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// setCurrentRecordInfo
//=============================
GTI_ANALYSIS_RETURN FloodControl::setCurrentRecordInfo (GTI_STRATEGY_TYPE direction, unsigned int channel)
{
    myCurDirection = direction;
    myCurChannel = channel;
    myCurWasReported = false;

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getCurrentRecordInfo
//=============================
GTI_ANALYSIS_RETURN FloodControl::getCurrentRecordInfo (GTI_STRATEGY_TYPE *outDirection, unsigned int *outChannel)
{
    if (outDirection)
        *outDirection = myCurDirection;

    if (outChannel)
    {
        if (myCurDirection == GTI_STRATEGY_INTRA)
            *outChannel = 0;
        else
            *outChannel = myCurChannel;
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// markCurrentRecordBad
//=============================
GTI_ANALYSIS_RETURN FloodControl::markCurrentRecordBad (void)
{
    if (myCurWasReported)
        return GTI_ANALYSIS_SUCCESS;

    myCurWasReported = true;
    unsigned int setValue = 0;
    std::list<PriorityListEntry>::iterator pos;

    switch (myCurDirection)
    {
    case GTI_STRATEGY_UP:
        if (myHasUp)
        {
            myUpState.numBad+=1;
            setValue = myUpState.numBad;
            pos = myUpState.priorityPos;
        }
        break;
    case GTI_STRATEGY_DOWN:
        if (myCurChannel < myDownStates.size())
        {
            myDownStates[myCurChannel].numBad+=1;
            setValue = myDownStates[myCurChannel].numBad;
            pos = myDownStates[myCurChannel].priorityPos;
        }
        break;
    case GTI_STRATEGY_INTRA:
        if (myHasIntra)
        {
            myIntraState.numBad+=1;
            setValue = myIntraState.numBad;
            pos = myIntraState.priorityPos;
        }
        break;
    }

    //Store the maxium badness
    if (setValue > myMaxBad)
    {
        myMaxBad = setValue;
        myMaxDirection = myCurDirection;
        myMaxChannel = myCurChannel;
    }

    //DEBUG
    //std::cout << "Mark as bad: " << myCurDirection << "@" << myCurChannel << std::endl;

    //Update priority list
    updatePriorityList (pos);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getCurrentTestDecision
//=============================
GTI_ANALYSIS_RETURN FloodControl::getCurrentTestDecision (gti::GTI_STRATEGY_TYPE *outDirection, unsigned int *outChannel)
{
    myUsePriority = true;

    while (!myNextDecision->state->enabled) //TODO detect if all channels are disabled, remove disabling after a certain amount of time also (if the threshold is too small)
    {
        nextTestDecision();
    }

    if (outDirection)
        *outDirection = myNextDecision->direction;
    myCurDirection = myNextDecision->direction;

    if (outChannel)
        *outChannel = myNextDecision->channel;
    myCurChannel = myNextDecision->channel;

    //DEBUG
    //std::cout << "GetCurrentTestDecision: " << myCurDirection << "@" << myCurChannel << std::endl;

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// nextTestDecision
//=============================
GTI_ANALYSIS_RETURN FloodControl::nextTestDecision (void)
{
    //Store that we had no success here
    	myNextDecision->state->numFailedTests+=1;

    //Next
    myNextDecision++;
    if (myNextDecision == myPriority.end())
        myNextDecision = myPriority.begin();

    //DEBUG
    //std::cout << "Next Decision" << std::endl;

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// rewindDecision
//=============================
GTI_ANALYSIS_RETURN FloodControl::rewindDecision (void)
{
    //DEBUG
    //std::cout << "Rewind Decision" << std::endl;

    //Update priorities for all entries that yielded no result
    std::list<PriorityListEntry>::iterator cur = myNextDecision, prev;
    while (cur != myPriority.begin() && cur != myPriority.end())
    {
        prev = cur;
        prev--;
        updatePriorityList (prev);
        cur--;
    }

    //Rewind
    myNextDecision = myPriority.begin();

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// updatePriorityList
//=============================
void FloodControl::updatePriorityList (std::list<PriorityListEntry>::iterator pos)
{
    //Addjust the priority list
    if (myUsePriority)
    {
        std::list<PriorityListEntry>::iterator cur = pos, next = cur;
        next++;

        while (next != myPriority.end() && next->state->numBad + next->state->numFailedTests < pos->state->numBad + pos->state->numFailedTests)
        {
            cur++;
            next++;
        }

        if (cur != pos)
        {
            StateInfo *state = pos->state;
            myPriority.splice (next, myPriority, pos);
            state->priorityPos = cur;
            state->priorityPos++;
        }

        //DEBUG
        /*
        std::list<PriorityListEntry>::iterator dbg;
        std::cout << "PRIORITY LIST (" << getpid() << "): ";
        for (dbg = myPriority.begin(); dbg != myPriority.end(); dbg++)
            std::cout << dbg->direction << "@" << dbg->channel << "=" << dbg->state->numBad + dbg->state->numFailedTests << ", ";
        std::cout << std::endl;

        std::cout << "LIST POSITIONS OF STATES: ";
        for (int i = 0; i < myDownStates.size(); i++)
        {
            int position = 0;
            for (dbg = myPriority.begin(); dbg != myPriority.end(); dbg++, position++)
            {
                if (dbg == myDownStates[i].priorityPos)
                    break;
            }

            if (dbg == myPriority.end())
                std::cout << "UNKNOWN, ";
            else
                std::cout << position;
        }
        std::cout << std::endl;
        */
        //END DEBBUG

        //Reset current decision
        myNextDecision = myPriority.begin();
    }
}

//=============================
// modifyQueueSize
//=============================
GTI_ANALYSIS_RETURN FloodControl::modifyQueueSize (GTI_STRATEGY_TYPE direction, unsigned int channel, int diff)
{
    StateInfo *selected = NULL;

    switch (direction)
    {
    case GTI_STRATEGY_UP:
        if (myHasUp)
        {
            myUpState.queueSize += diff;
            selected = &myUpState;
        }
        break;
    case GTI_STRATEGY_DOWN:
        if (channel < myDownStates.size())
        {
            myDownStates[channel].queueSize += diff;
            selected = &(myDownStates[channel]);
        }
        break;
    case GTI_STRATEGY_INTRA:
        if (myHasIntra)
        {
            myIntraState.queueSize += diff;
            selected = &myIntraState;
        }
        break;
    }

    if (selected)
    {
        if (selected->enabled && selected->queueSize >= DISABLE_THRESHOLD)
        {
            //TODO this feature is experimental and currently disabled
            //selected->enabled = false;
        }
        if (!selected->enabled && selected->queueSize < DISABLE_THRESHOLD - ENABLE_HISTERESE)
        {
            //TODO this feature is experimental and currently disabled
            //selected->enabled = true;
        }
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getMaxBadness
//=============================
uint64_t FloodControl::getMaxBadness (void)
{
    return myMaxBad;
}

/*EOF*/
