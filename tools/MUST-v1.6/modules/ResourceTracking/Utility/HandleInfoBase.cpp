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
 * @file HandleInfoBase.h
 *       @see HandleInfoBase.
 *
 *  @date 23.06.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "HandleInfoBase.h"

#include <assert.h>
#include <iostream>
#include <sstream>

using namespace must;

bool HandleInfoBase::ourAllowFreeForwarding = true;

#ifdef MUST_DEBUG
std::map<std::string, std::map<HandleInfoBase*,bool> > HandleInfoBase::ourHandles = std::map<std::string, std::map<HandleInfoBase*,bool> > ();
int HandleInfoBase::ourNumTrackers = 0;
#endif

//=============================
// Constructor
//=============================
HandleInfoBase::HandleInfoBase (std::string resourceName)
 : userRefCount (0),
   mpiRefCount (1),
   myForwardedToPlaces (),
   myPassFreeAcross (NULL)
{
    //Usually nothing to do
#ifdef MUST_DEBUG
    /*    std::map<std::string, std::map<HandleInfoBase*,bool> >::iterator pos;
   pos = ourHandles.find (getResourceName());
    if (pos == ourHandles.end())
    {
        ourHandles.insert (std::make_pair(getResourceName(), std::map<HandleInfoBase*, bool> () ));
    }*/

    ourHandles[resourceName].insert (std::make_pair(this, true));
#endif
}

//=============================
// Destructor
//=============================
HandleInfoBase::~HandleInfoBase ()
{
    //Nothing to do ?
    assert (userRefCount <= 0 && mpiRefCount <= 0);
}

//=============================
// erase
//=============================
bool HandleInfoBase::erase (void)
{
    userRefCount--;
    if (userRefCount <= 0 && mpiRefCount <= 0)
    {
        deleteThis();
        return true;
    }

    if (userRefCount == 0)
        return true;
    return false;
}

//=============================
// copy
//=============================
bool HandleInfoBase::copy (void)
{
    incRefCount ();
    return true;
}

//=============================
// mpiDestroy
//=============================
bool HandleInfoBase::mpiDestroy (void)
{
    mpiRefCount=0;

    if (userRefCount <= 0)
    {
        deleteThis();
        return true;
    }
    return false;
}

//=============================
// incRefCount
//=============================
void HandleInfoBase::incRefCount (void)
{
    assert (userRefCount >= 0);
    userRefCount++;
}

//=============================
// mpiIncRefCount
//=============================
void HandleInfoBase::mpiIncRefCount (void)
{
    assert (mpiRefCount >= 0);
    mpiRefCount++;
}

//=============================
// mpiErase
//=============================
bool HandleInfoBase::mpiErase (void)
{
    mpiRefCount--;
    if (userRefCount <= 0 && mpiRefCount <= 0)
    {
        deleteThis();
        return true;
    }

    if (mpiRefCount == 0)
        return true;
    return false;
}

//=============================
// subscribeTracker
//=============================
void HandleInfoBase::subscribeTracker (void)
{
#ifdef MUST_DEBUG
    ourNumTrackers++;
#endif
}

//=============================
// unsubscribeTracker
//=============================
void HandleInfoBase::unsubscribeTracker (void)
{
#ifdef MUST_DEBUG
    ourNumTrackers--;

    if (ourNumTrackers==0)
        printLostHandles ();
#endif
}

//=============================
// printLostHandles
//=============================
void HandleInfoBase::printLostHandles (void)
{
#ifdef MUST_DEBUG
    std::map<std::string, std::map<HandleInfoBase*,bool> >::iterator iter;
    for (iter = ourHandles.begin(); iter != ourHandles.end(); iter++)
    {
        if (!iter->second.size()) continue;

        std::cout << "Listing lost " << iter->first << " informations:" << std::endl;
        int i = 1;

        std::map<HandleInfoBase*, bool>::iterator infIter;
        for (infIter = iter->second.begin(); infIter != iter->second.end(); infIter++, i++)
        {
            std::list<std::pair<MustParallelId,MustLocationId> > references;
            std::stringstream stream;
            infIter->first->printInfo (stream, &references);
            std::cout << i << ": " << stream.str ();

            std::list<std::pair<MustParallelId,MustLocationId> >::iterator refIter;
            int j;
            for (refIter = references.begin(); refIter != references.end(); refIter++, j++)
            {
                std::cout << " reference " << j << ": " << refIter->first << "#" << refIter->second;
            }
            std::cout << std::endl;
        }
    }
#endif
}

//=============================
// deleteThis
//=============================
void HandleInfoBase::deleteThis (void)
{
#ifdef MUST_DEBUG
    std::map<HandleInfoBase*, bool>::iterator pos = ourHandles[getResourceName()].find (this);
    if (pos != ourHandles[getResourceName()].end())
        ourHandles[getResourceName()].erase (pos);
#endif

    //Notify all remote places of the destruction of this resource
    std::set<std::pair<int, int> >::iterator forwardIter;

    if (myPassFreeAcross)
    {
        for (forwardIter = myForwardedToPlaces.begin(); forwardIter != myForwardedToPlaces.end(); forwardIter++)
        {
            if (ourAllowFreeForwarding)
                (*myPassFreeAcross) (forwardIter->second, getRemoteId(), forwardIter->first);
        }
    }

    delete this;
}

//=============================
// setForwardedToPlace
//=============================
void HandleInfoBase::setForwardedToPlace (int placeId, int rank, passFreeAcrossP freeFunction)
{
    myForwardedToPlaces.insert(std::make_pair(placeId, rank));

    if (!myPassFreeAcross)
        myPassFreeAcross = freeFunction;
}

//=============================
// wasForwardedToPlace
//=============================
bool HandleInfoBase::wasForwardedToPlace (int placeId, int rank)
{
    return (myForwardedToPlaces.find(std::make_pair(placeId, rank)) != myForwardedToPlaces.end());
}

//=============================
// getRemoteId
//=============================
uint64_t HandleInfoBase::getRemoteId (void)
{
    return (uint64_t)(void*)this;
}

//=============================
// disableFreeForwardingAcross
//=============================
void HandleInfoBase::disableFreeForwardingAcross (void)
{
    ourAllowFreeForwarding = false;
}

/*EOF*/
