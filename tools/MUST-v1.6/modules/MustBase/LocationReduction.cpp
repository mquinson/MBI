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
 * @file LocationReduction.cpp
 *       @see LocationReduction
 *
 *  @date 11.01.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "LocationReduction.h"
#include "BaseApi.h"

using namespace must;

mGET_INSTANCE_FUNCTION(LocationReduction)
mFREE_INSTANCE_FUNCTION(LocationReduction)
mPNMPI_REGISTRATIONPOINT_FUNCTION(LocationReduction)

//=============================
// LocationReduction
//=============================
LocationReduction::LocationReduction (const char* instanceName)
    : gti::ModuleBase<LocationReduction, I_LocationReduction> (instanceName),
      myLocationModule (NULL)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Needs a sub module for as location module
    assert (subModInstances.size() >= 1);

    myLocationModule = (I_LocationAnalysis*) subModInstances[0];
}

//=============================
// ~LocationReduction
//=============================
LocationReduction::~LocationReduction (void)
{
	if (myLocationModule)
		destroySubModuleInstance ((I_Module*) myLocationModule);
	myLocationModule = NULL;
}

//=============================
// reduce
//=============================
gti::GTI_ANALYSIS_RETURN LocationReduction::reduce (
    		MustParallelId pId,
    		MustLocationId lId,
    		char* callName,
    		int callNameLen,
#ifdef USE_CALLPATH
    		int numStackLevels,
    		int stackInfosLength,
    		int indicesLength,
    		int* infoIndices,
    		char* stackInfos,
#endif
    		gti::I_ChannelId *thisChannel,
    		std::list<gti::I_ChannelId*> *outFinishedChannels)
{
	/*
	 * We need not touch "thisChannel" and "outFinishedChannels"
	 * as we never return WAITING.
	 */

    //Make sure we kill the upper 32 bit that hold the occurrence count, we do not care about that here!
    lId = (lId & 0x00000000FFFFFFFF);

	//== Build given location
	LocationInfo info;
	info.callName = callName;

	//== Query the location analysis for this location
	LocationInfo current = myLocationModule->getInfoForId(pId, lId);

	//== If this location is already known. we filter this record out (no replacement record needed)
	if (current.callName == info.callName
#ifdef USE_CALLPATH
	       /**
	        * @todo we never filter out with callpaths right now, we need to extend this in order to make it work in
	        * that case. The Location implmenentation must be channel-id aware and must use a map of channel ids
	        * to location infos instead of the global map.
	        */
	      && false
#endif
	    )
	{
#ifdef MUST_DEBUG
		std::cout << "DEBUG: Reduced location with id " << lId << " from parallel id " << pId << std::endl;
#endif
		return GTI_ANALYSIS_SUCCESS;
	}

	//== If locations differ, we must not filter out
	/*
	 * Important rule for filters:
	 *  one is tempted to simply return GTI_ANALYSIS_IRREDUCIBLE here, but this is a bad idea.
	 *  The dominant(not filtered) records must have priority here, otherwise we will break the
	 *  order property.
	 *  Above we filter out records with not calling the wrapp-everywhere call and returning
	 *  GTI_ANALYSIS_SUCCESS. This causes the record to be dropped completly, however
	 *  if the record that carried the redundant data did not got a channel id identifiing this
	 *  place, it may happen that records after the reduced out record will be processed before
	 *  the redundant record arrives. So if this place has two connected places A and B and
	 *  first from A a location arrives (L1) that is not filtered out. Then, if afterwards from B
	 *  a redundant location (L2) arrives and afterwards a record (R) that uses this location. Than
	 *  the location from B is dropped and the record forwarded. Now at the place that receives
	 *  records from this place, if a suspension is going on. Then at this place in one queue
	 *  L1 would be enqueued and in the other one R. When the suspension ends, the place
	 *  is free to process R first. This breaks order as L1 would have had to be processed first.
	 *  So the records we don't filter out must be created with the wrapp-everywhere call
	 *  and a return of GTI_ANALYSIS_SUCCESS.
	 */
	handleNewLocationP fP;
	if (getWrapperFunction ("handleNewLocation", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
	{
		(*fP) (
				pId,
				lId,
				callName,
				callNameLen
#ifdef USE_CALLPATH
				,
				numStackLevels,
				stackInfosLength,
				indicesLength,
				infoIndices,
				stackInfos
#endif
		);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// timeout
//=============================
void LocationReduction::timeout (void)
{
	//Nothing to do, we never return the state "waiting"
}

/*EOF*/
