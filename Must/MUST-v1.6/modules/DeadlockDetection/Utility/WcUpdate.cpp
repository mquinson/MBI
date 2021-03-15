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
 * @file WcUpdate.cpp
 *       @see MUST::WcUpdate.
 *
 *  @date 15.03.2011
 *  @author Tobias Hilbrich, Mathias Korepkat
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "DeadlockApi.h"

#include "WcUpdate.h"

using namespace must;

mGET_INSTANCE_FUNCTION(WcUpdate)
mFREE_INSTANCE_FUNCTION(WcUpdate)
mPNMPI_REGISTRATIONPOINT_FUNCTION(WcUpdate)

//=============================
// Constructor
//=============================
WcUpdate::WcUpdate (const char* instanceName)
    : gti::ModuleBase<WcUpdate, I_WcUpdate> (instanceName),
      myWcReqs(),
      myAnySource (-1)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUB_MODS 1
    if (subModInstances.size() < NUM_SUB_MODS)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
    if (subModInstances.size() > NUM_SUB_MODS)
    {
    		for (std::vector<I_Module*>::size_type i = NUM_SUB_MODS; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }

    myRTrack = (I_RequestTrack*) subModInstances[0];

    //Initialize module data
    /*Nothing to do*/
}

//=============================
// Destructor
//=============================
WcUpdate::~WcUpdate ()
{
	if (myRTrack)
		destroySubModuleInstance ((I_Module*) myRTrack);
	myRTrack = NULL;

	myWcReqs.clear();
}

//=============================
// addPredefineds
//=============================
GTI_ANALYSIS_RETURN WcUpdate::addPredefineds (
    			int anySource)
{
	myAnySource = anySource;
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// recvPost
//=============================
GTI_ANALYSIS_RETURN WcUpdate::recvPost (
		MustParallelId pId,
		MustLocationId lId,
		int source,
		int statusSource)
{
	if (source == myAnySource)
	{
		propagateRecvUpdateP fP;
		if (getWrapperFunction ("propagateRecvUpdate", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
		{
			(*fP) (
					pId,
					lId,
					statusSource
				);
		}
	}
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// irecv
//=============================
GTI_ANALYSIS_RETURN WcUpdate::irecv (
		MustParallelId pId,
		MustLocationId lId,
		int source,
		MustRequestType request)
{
	//Only remember wildcard receives
	if (source != myAnySource)
		return GTI_ANALYSIS_SUCCESS;

	//Find the right request map
	AllMaps::iterator m = myWcReqs.find (pId);
	if (m == myWcReqs.end())
	{
		myWcReqs.insert (std::make_pair(pId, RequestMap()));
		m = myWcReqs.find (pId);
	}

	//Add to the request map
	m->second.insert(std::make_pair(request, source));

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// startPersistent
//=============================
GTI_ANALYSIS_RETURN WcUpdate::startPersistent (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request)
{
	I_Request* info = myRTrack->getRequest(pId, request);

	//Make sure it is persistent and known
	if (info == NULL || !info->isPersistent())
		return GTI_ANALYSIS_SUCCESS;

	//We don't care about sends
	if (info->isSend())
		return GTI_ANALYSIS_SUCCESS;

	//Get the persistent recv info
	if (info->getSource() != myAnySource)
		return GTI_ANALYSIS_SUCCESS;

	//Find the right request map
	AllMaps::iterator m = myWcReqs.find (pId);
	if (m == myWcReqs.end())
	{
		myWcReqs.insert (std::make_pair(pId, RequestMap()));
		m = myWcReqs.find (pId);
	}

	//Add to the request map
	m->second.insert(std::make_pair(request, info->getSource()));

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// startPersistentArray
//=============================
GTI_ANALYSIS_RETURN WcUpdate::startPersistentArray (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType* requests,
		int count)
{
	for (int i = 0; i < count; i++)
	{
		GTI_ANALYSIS_RETURN ret = startPersistent (pId, lId, requests[i]);
		if (ret != GTI_ANALYSIS_SUCCESS)
			return ret;
	}
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// complete
//=============================
GTI_ANALYSIS_RETURN WcUpdate::complete (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request,
		int flag,
		int statusSource)
{
	//Is the flag true ?
	if (!flag) return GTI_ANALYSIS_SUCCESS;

	//Are there any requests for this pid ?
	AllMaps::iterator m = myWcReqs.find(pId);
	if (m == myWcReqs.end()) return GTI_ANALYSIS_SUCCESS;

	//Is this request in the map
	RequestMap::iterator r = m->second.find(request);
	if (r == m->second.end()) return GTI_ANALYSIS_SUCCESS;

	//Remove the request and create the update
	m->second.erase(r);

	propagateIrecvUpdateP fP;
	if (getWrapperFunction ("propagateIrecvUpdate", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
	{
		(*fP) (
				pId,
				lId,
				statusSource,
				request
			);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// completeAny
//=============================
GTI_ANALYSIS_RETURN WcUpdate::completeAny (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType* requests,
		int count,
		int index,
		int flag,
		int statusSource)
{
	if (!flag || count<=0 || index<0 || index>=count)
		return GTI_ANALYSIS_SUCCESS;

	return complete (pId, lId, requests[index], true, statusSource);
}

//=============================
// completeArray
//=============================
GTI_ANALYSIS_RETURN WcUpdate::completeArray (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType *requests,
		int count,
		int flag,
		int* statusSources)
{
	if (!flag || count<=0)
		return GTI_ANALYSIS_SUCCESS;

	for (int i = 0; i < count; i++)
	{
		GTI_ANALYSIS_RETURN ret = complete (pId, lId, requests[i], true, statusSources[i]);
		if (ret != GTI_ANALYSIS_SUCCESS)
			return ret;
	}
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// completeSome
//=============================
GTI_ANALYSIS_RETURN WcUpdate::completeSome (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType *requests,
		int count,
		int *indices,
		int numIndices,
		int* statusSources)
{
    if (count<=0 || numIndices<=0 || count<numIndices)
        return GTI_ANALYSIS_SUCCESS;

	for (int i = 0; i < numIndices; i++)
	{
		GTI_ANALYSIS_RETURN ret = complete (pId, lId, requests[indices[i]], true, statusSources[i]);
		if (ret != GTI_ANALYSIS_SUCCESS)
			return ret;
	}

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
