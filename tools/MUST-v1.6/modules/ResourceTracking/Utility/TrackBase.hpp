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
 * @file TrackBase.hpp
 *       @see MUST::TrackBase.
 *
 *  @date 10.02.2011
 *  @author Tobias Hilbrich
 */

//=============================
// Constructor
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::TrackBase (const char* instanceName)
: ModuleBase<SUPER, INTERFACE> (instanceName),
  myNullValue (0),
  myNullInfo (NULL),
  myPredefineds (),
  myUserHandles (),
  myLastQuery (),
  myRemoteRes (),
  myPIdMod (NULL),
  myLIdMod (NULL),
  myFurtherMods ()
  {
    HandleInfoBase::subscribeTracker();

	//create sub modules
	std::vector<I_Module*> subModInstances;
	subModInstances = ModuleBase<SUPER, INTERFACE>::createSubModuleInstances ();

	//handle sub modules
	if (subModInstances.size() < 2)
	{
		std::cerr << "ERROR: " << __FILE__ << "@" << __LINE__ << " needs one sub module as parallel id module and one as location if module." << std::endl;
		assert (0);
	}
	myFurtherMods.resize(subModInstances.size()-2);
	for (std::vector<I_Module*>::size_type i = 2; i < subModInstances.size(); i++)
	{
		myFurtherMods[i-2] = subModInstances[i];
	}

	myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
	myLIdMod = (I_LocationAnalysis*) subModInstances[1];

	//initialize
	myLastQuery = myUserHandles.end();
  }

//=============================
// Destructor
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::~TrackBase (void)
{
    //Notify HandleInfo Base that no frees should be passed across anymore
    HandleInfoBase::disableFreeForwardingAcross();

	//destroy ParallelId module
	if (myPIdMod)
		ModuleBase<SUPER, INTERFACE>::destroySubModuleInstance ((I_Module*) myPIdMod);
	myPIdMod = NULL;

	//destroy LocationlId module
	if (myLIdMod)
	    ModuleBase<SUPER, INTERFACE>::destroySubModuleInstance ((I_Module*) myLIdMod);
	myLIdMod = NULL;

	//destroy any other modules
	for (std::vector<I_Module*>::size_type i = 0; i < myFurtherMods.size(); i++)
	{
		if (myFurtherMods[i])
			ModuleBase<SUPER, INTERFACE>::destroySubModuleInstance (myFurtherMods[i]);
		myFurtherMods[i] = NULL;
	}
	myFurtherMods.clear();

	freeHandleMaps ();

	HandleInfoBase::unsubscribeTracker();
}

//=============================
// addPredefineds
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
GTI_ANALYSIS_RETURN TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::addPredefineds (
        MustParallelId pId,
        HANDLE_TYPE nullValue,
        int numPredefs,
        int* predefinedIds,
        HANDLE_TYPE* predefinedValues
)
{
    return addPredefineds(pId2Rank(pId), nullValue, numPredefs, predefinedIds, predefinedValues);
}
//=============================
// addPredefineds
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
GTI_ANALYSIS_RETURN TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::addPredefineds (
        int rank,
		HANDLE_TYPE nullValue,
		int numPredefs,
		int* predefinedIds,
		HANDLE_TYPE* predefinedValues
)
{
	static bool wasAdded = false;
    FULL_INFO * info;
    if (wasAdded)
    {
        if (myNullValue != nullValue)
        {
            myNullValues.insert(std::make_pair(rank, nullValue));
        }
        for (int i = 0; i < numPredefs; i++)
        {
            typename PredefinedMap::iterator predef = myPredefinedMap.find(predefinedIds[i]);
            if (predef == myPredefinedMap.end())
            { // should not happen?
                //assert(0);
                info = createPredefinedInfo (predefinedIds[i], predefinedValues[i]);
                myPredefineds.insert (std::make_pair(predefinedValues[i], info));
                myPredefinedMap.insert(std::make_pair(predefinedIds[i], std::make_pair(predefinedValues[i], info)));
            }
            else if (predef->second.first != predefinedValues[i])
            {
                myUserHandles.insert( std::make_pair( std::make_pair( rank, predefinedValues[i]), predef->second.second));
                if (predef->second.second)
                {
                    predef->second.second->copy();
                }
            }
        }
    }
    else
    {
        //store the null value
        myNullValue = nullValue;
        // enough to have one predefined info object?
        myNullInfo = createPredefinedInfo(0, nullValue);
        //store the other predefined values
        for (int i = 0; i < numPredefs; i++)
        {
            info = createPredefinedInfo (predefinedIds[i], predefinedValues[i]);
            myPredefineds.insert (std::make_pair(predefinedValues[i], info));
            myPredefinedMap.insert(std::make_pair(predefinedIds[i],std::make_pair(predefinedValues[i],info)));
        }
    }



    wasAdded = true;
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// findUserHandle
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
typename TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::HandleMap::iterator
TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::findUserHandle (
        MustParallelId pId, HANDLE_TYPE handle)
{
    return findUserHandle (pId2Rank(pId), handle);
}

//=============================
// findUserHandle
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
typename TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::HandleMap::iterator
TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::findUserHandle (
        int rank, HANDLE_TYPE handle)
{
    //Look at last query (avoid the search if that was already the right request)
    if (    myLastQuery != myUserHandles.end()  &&
            myLastQuery->first.first == rank &&
            myLastQuery->first.second == handle)
        return myLastQuery;

    myLastQuery = myUserHandles.find (std::make_pair(rank, handle));

    return myLastQuery;
}

//=============================
// getHandleInfo
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
FULL_INFO* TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::getHandleInfo (MustParallelId pId, HANDLE_TYPE handle)
{
    return getHandleInfo (pId2Rank(pId), handle);
}

//=============================
// getHandleInfo
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
FULL_INFO* TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::getHandleInfo (int rank, HANDLE_TYPE handle)
{
    FULL_INFO* ret;

    // is it MPI_handle_NULL ?
    if (!myNullValues.empty() && myNullValues.find(rank) != myNullValues.end())
    { // rank specific NULL value available
        if (myNullValues[rank] == handle)
        {
            return myNullInfo;
        }
    }
    else if (handle == myNullValue)
    { // global NULL value
        return myNullInfo;
    }

    //Is it a user defined type ?
    if (    myLastQuery == myUserHandles.end()  ||
            myLastQuery->first.first != rank ||
            myLastQuery->first.second != handle)
    {
        myLastQuery = myUserHandles.find (std::make_pair(rank, handle));
    }

    if (myLastQuery == myUserHandles.end())
    {
        //If not a user type-> it must a predefined

        typename PredefinedInfos::iterator prePos = myPredefineds.find(handle);

        if (prePos == myPredefineds.end())
            return NULL;

        ret = prePos->second;
    }
    else
    {
        ret = myLastQuery->second;
    }

    return ret;
}

//=============================
// isAlreadyKnown
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
bool TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::isAlreadyKnown (
		MustParallelId pId, HANDLE_TYPE handle)
{
#ifdef MUST_DEBUG
	//Known as non-predefined request ?
	typename HandleMap::iterator pos = myUserHandles.find (std::make_pair(pId2Rank(pId), handle));
	if (pos != myUserHandles.end())
	{
		//This is more of an internal error, we log it directly to std::cerr
		std::cerr << "Error: in " << __FILE__ << __LINE__ << " tried to add a user handle that is still present, implementation error in MPI or MUST." << std::endl;
		return true;
	}

	//Is null
	if (!myNullValues.empty() && myNullValues.find(pId2Rank(pId)) != myNullValues.end() || handle == myNullValue)
	{
		//This is more of an internal error, we log it directly to std::cerr
		std::cerr << "Error: in " << __FILE__ << __LINE__ << " tried to add a handle that had the value of the null handle, implementation error in MPI or MUST." << std::endl;
		return true;
	}

	//Is it a predefined value
	typename PredefinedInfos::iterator prePos = myPredefineds.find (handle);
	if (prePos != myPredefineds.end())
	{
		//This is more of an internal error, we log it directly to std::cerr
		std::cerr << "Error: in " << __FILE__ << __LINE__ << " tried to add a user handle that has the value of a predefined handle, implementation error in MPI or MUST." << std::endl;
		return true;
	}
#endif /*MUST_DEBUG*/

	return false;
}

//=============================
// submitUserHandle
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
bool TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::submitUserHandle (
        MustParallelId pId, HANDLE_TYPE handle, FULL_INFO* handleInfo)
{
    return submitUserHandle (pId2Rank(pId), handle, handleInfo);
}

//=============================
// submitUserHandle
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
bool TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::submitUserHandle (
        int rank,
        HANDLE_TYPE handle,
        FULL_INFO* handleInfo)
{
    std::pair<typename HandleMap::iterator, bool> ret = 
            myUserHandles.insert(
                    std::make_pair(
                            std::make_pair(rank, handle),
                            handleInfo
                    )
            );

    if (!ret.second)
    {
        //Ok, there is already a handle with same value in there! We need to overwrite
        //Kill old
        myUserHandles.erase (ret.first);

#ifdef MUST_DEBUG
        //Thats rather weird, we should warn!
        std::cout << "Warning: a user handle was added that was already in our user handle map, we overwrote the old one with the new one, but should that really happen? " << __FILE__<<":"<<__LINE__<<std::endl;
#endif /*MUST_DEBUG*/

        //Now again
        ret = myUserHandles.insert(
                std::make_pair(
                        std::make_pair(rank, handle),
                        handleInfo
                )
        );
    }

    // refCount initialized with 1 in constructor
    myLastQuery = ret.first;
    return ret.second;
}

//=============================
// removeUserHandle
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
bool TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::removeUserHandle (
        MustParallelId pId, HANDLE_TYPE handle)
{
    return removeUserHandle (pId2Rank(pId), handle);
}

//=============================
// removeUserHandle
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
bool TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::removeUserHandle (
        int rank,
        HANDLE_TYPE handle)
{
    typename HandleMap::iterator pos = findUserHandle (rank, handle);
    if (pos == myUserHandles.end())
        return false;

    // decrease MPI refCount (Remove from list if last mpi ref count was removed of no info is present at all)
    if (!pos->second || pos->second->mpiErase())
    {
        myUserHandles.erase(pos);
        myLastQuery = myUserHandles.end();
    }

    return true;
}

//=============================
// submitRemoteResource
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
bool TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::submitRemoteResource (
        int rank,
        MustRemoteIdType remoteId,
        bool hasHandle,
        HANDLE_TYPE handle,
        FULL_INFO* handleInfo)
{
    //If we have a handle we need to add it to the user handles as well
    if (hasHandle)
    {
        submitUserHandle (rank, handle, handleInfo);
    }

    //Now add it to the Remote resources
    RemoteIdentifier id = std::make_pair (rank, remoteId);
    RemoteResourceInfo info = std::make_pair (handleInfo, std::make_pair(hasHandle, handle));
    
    myRemoteRes[id] = info;

    return true;
}

//=============================
// removeRemoteHandle
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
bool TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::removeRemoteResource (
        int rank,
        MustRemoteIdType remoteId)
{
    RemoteIdentifier id = std::make_pair(rank, remoteId);
    typename RemoteMap::iterator pos = myRemoteRes.find (id);

    if (pos == myRemoteRes.end())
        //Does not exists, should we warn?
        return false;

    bool hasHandle = pos->second.second.first;
    HANDLE_TYPE handle = pos->second.second.second;
    FULL_INFO* resource = pos->second.first;

    if (hasHandle)
    {
        //This removes the resource from the user handles AND it kills the resource (should have a MPI ref count of exactly 1)
        removeUserHandle (rank, handle);
    }
    else
    {
        //We need to do the mpi erase ourselves
        if (resource) resource->mpiErase();
    }

    //Delete from the remote resource map
    myRemoteRes.erase (pos);

    return true;
}

//=============================
// getRemoteIdInfo
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
FULL_INFO* TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::getRemoteIdInfo (
        int rank,
        MustRemoteIdType remoteId)
{
    RemoteIdentifier id = std::make_pair (rank, remoteId);
    typename RemoteMap::iterator pos = myRemoteRes.find (id);

    if (pos == myRemoteRes.end())
        return NULL; //No such remote resource exists

    return pos->second.first;
}

//=============================
// pId2Rank
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
int TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::pId2Rank (
		MustParallelId pId)
{
	return myPIdMod->getInfoForId(pId).rank;
}

//=============================
// createPredefinedInfo
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
FULL_INFO *
TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::createPredefinedInfo (int value, HANDLE_TYPE handle)
{
	//return an invalid info
    return NULL;
}

//=============================
// getUserHandles
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
std::list<std::pair<int, HANDLE_TYPE> >
TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::getUserHandles (void)
{
	typename std::list<std::pair<int, HANDLE_TYPE> > handles;

	typename HandleMap::iterator i;
	for (i = myUserHandles.begin(); i != myUserHandles.end(); i++)
	{
	    if(!this->isPredefined((I_INFO*)i->second))
		handles.push_back(std::make_pair(i->first.first, i->first.second));
	}

	return handles;
}

//=============================
// freeHandleMaps
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
void
TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::freeHandleMaps ()
{
    //free data
    /*
     * We iterate over all handles that are still in out predefined/user handle list.
     * We perform an mpi destroy on each remaining entry:
     * - If the destroy returns that
     *    the object still exist, then some module forgot to erase a persistent
     *    info.
     *
     * Important:
     * If we have such forgotten handles, we simply not free them, we must also
     * no call the "user destroy" function of HandleInfoBase! As handles may have
     * dependencies between each other, freeing some handle before another
     * may cause segfaults or assertions. So we just leave them to exist (which
     * only happens in an error case anyways).
     */
    ////User
    typename HandleMap::iterator userIter = myUserHandles.begin();
    for (; userIter != myUserHandles.end(); userIter++)
    {
        if (!userIter->second) continue;

        if(!userIter->second->mpiDestroy())
        {
#ifdef MUST_DEBUG
            std::cout << "Warning: A module forgot to erase a persistent handle (it was also not freed by the application, thats why we saw this ...)." << std::endl;
#endif
        }
    }

    ////Predefined
    typename PredefinedInfos::iterator preIter = myPredefineds.begin();
    for (; preIter != myPredefineds.end(); preIter++)
    {
        if (!preIter->second) continue;

        if(!preIter->second->mpiDestroy())
        {
#ifdef MUST_DEBUG
           std::cout << "Warning: some module failed to erase a persistent handle, you should breakpoint here and investigate." << std::endl;
#endif
        }
    }

    ////Remote
    typename RemoteMap::iterator remoteIter = myRemoteRes.begin();
    for (; remoteIter != myRemoteRes.end(); remoteIter++)
    {
        RemoteResourceInfo info = remoteIter->second;
        if (!info.first) continue; //anything in there?
        if (info.second.first) continue; //If it had a handle we freed it in the user handle map already!

        if(!info.first->mpiDestroy())
        {
#ifdef MUST_DEBUG
            std::cout << "Warning: some module failed to erase a persistent handle on a remote resource, you should breakpoint here and investigate." << std::endl;
#endif
        }
    }

    ////Null
    if (myNullInfo)
        myNullInfo->mpiDestroy();
    myNullInfo = NULL;

    myPredefineds.clear();
    myUserHandles.clear();
    myRemoteRes.clear();
}

//=============================
// getHandleForInfo
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
bool TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::getHandleForInfo (
        int rank,
        FULL_INFO* info,
        HANDLE_TYPE *pOutHandle)
{
    //Is it NULL ?
    if (info == myNullInfo)
    {
        if (!myNullValues.empty() && myNullValues.find(rank) != myNullValues.end())
        {
            if (pOutHandle) *pOutHandle = myNullValues[rank];
        }
        else
        {
            if (pOutHandle) *pOutHandle = myNullValue;
        }
        return true;
    }


    //Is it a user handle?
    //TODO we could speed this up by making HandleMap a two leveled map (rank->map<handle, info>)  O(p*n) -> O( ln(p) + n )
    // two leveled map is not that trivial in that case: crashes the caching + test for !=myUserHandles.end()!
    // O(p*n) is even worse in case of rank-based predefineds
    typename HandleMap::iterator userIter;
    for (userIter = myUserHandles.begin(); userIter != myUserHandles.end(); userIter++)
    {
        if (userIter->first.first != rank)
            continue;

        if (userIter->second == info)
        {
            if (pOutHandle) *pOutHandle = userIter->first.second;
            return true;
        }
    }

        //Is it a (global-constant) predefined ?
    typename PredefinedInfos::iterator preIter;
    for (preIter = myPredefineds.begin(); preIter != myPredefineds.end(); preIter++)
    {
        if (info == preIter->second)
        {
            if (pOutHandle) *pOutHandle = preIter->first;
            return true;
        }
    }

    return false;
}

//=============================
// notifyOfShutdown
//=============================
template <typename FULL_INFO, typename I_INFO, typename HANDLE_TYPE, typename PREDEFINED_ENUM, class SUPER, class INTERFACE>
void TrackBase<FULL_INFO, I_INFO, HANDLE_TYPE, PREDEFINED_ENUM, SUPER, INTERFACE>::notifyOfShutdown (void)
{
    HandleInfoBase::disableFreeForwardingAcross();
}

/*EOF*/
