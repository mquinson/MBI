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
 * @file CommReduction.cpp
 *       @see MUST::CommReduction.
 *
 *  @date 04.03.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "ResourceApi.h"

#include "CommReduction.h"

using namespace must;

mGET_INSTANCE_FUNCTION(CommReduction)
mFREE_INSTANCE_FUNCTION(CommReduction)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CommReduction)

//=============================
// Constructor
//=============================
CommReduction::CommReduction (const char* instanceName)
    : gti::ModuleBase<CommReduction, I_CommReduction> (instanceName),
      maxRank (-1),
      minRank (-1),
      myReductionPartners (),
      myCompletion (NULL),
      myTimedOut (false),
      myWasSuccessful (false)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    if (subModInstances.size() > 0)
    {
        for (std::vector<I_Module*>::size_type i = 0; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }

    //Initialize module data
    //nothing to do
}

//=============================
// Destructor
//=============================
CommReduction::~CommReduction ()
{
	//Clear completion tree
	if (myCompletion)
		delete myCompletion;
	myCompletion = NULL;

	//Clear all outstanding reduction partners, there should be none in common situations
	std::list<I_ChannelId*>::iterator iter;
	for (iter = myReductionPartners.begin(); iter != myReductionPartners.end(); iter++)
	{
		if (*iter)
			delete (*iter);
	}
	myReductionPartners.clear();
}

//=============================
// reduce
//=============================
GTI_ANALYSIS_RETURN CommReduction::reduce (
                    MustParallelId pId,
    				int reachableBegin,
    				int reachableEnd,
    				int worldSize,
    				MustCommType commNull,
    				MustCommType commSelf,
    				MustCommType commWorld,
    				int numWorlds,
    				MustCommType *worlds,
    				gti::I_ChannelId *thisChannel,
    				std::list<gti::I_ChannelId*> *outFinishedChannels)
{
	//Initialize completion tree
	if (!myCompletion)
		myCompletion = new CompletionTree (
				thisChannel->getNumUsedSubIds()-1, thisChannel->getSubIdNumChannels(thisChannel->getNumUsedSubIds()-1));

	//Did the reduction already time-out ?
	if (myTimedOut)
	{
		//Nothing to do here
		return GTI_ANALYSIS_IRREDUCIBLE;
	}

	//Handle the new data
	/**
	 * @todo we should detect non contiguous intervals here
	 *            this ensures detection of communication mechanisms
	 *            that do not connect successive ranks with each other.
	 *            The MPI based communication mechanism used
	 *            primarily right now does ensure this.
	 */
	if (reachableBegin < minRank || minRank < 0)
		minRank = reachableBegin;

	if (reachableEnd > maxRank  || maxRank < 0)
		maxRank = reachableEnd;

	for (int i = reachableBegin; i <= reachableEnd && i - reachableBegin < numWorlds; i++)
	{
		myWorlds.insert (std::make_pair(i, worlds[i-reachableBegin]));
	}

	//Add to tree and see whether we are completed
	myCompletion->addCompletion(thisChannel);

	if (myCompletion->isCompleted())
	{
		//Reset completions tree
		myCompletion->flushCompletions();

		//Add reduction partners to outFinishedChannels list
		std::list<I_ChannelId*>::iterator i;
		for (i = myReductionPartners.begin(); i != myReductionPartners.end(); i++)
		{
			//set as reopened channel
			outFinishedChannels->push_back (*i);
		}

		//Important: clear stored partners ... (do not delete their channel ids, this is done by the caller, as they are in the list of outFinishedChannels)
		myReductionPartners.clear();

		//Organize MPI_COMM_WORLD values
		int allNumWorlds = myWorlds.size();
		MustCommType *allWorlds = new MustCommType[allNumWorlds];
		int index = 0;
		std::map<int, MustCommType>::iterator iter;
		for (iter = myWorlds.begin(); iter != myWorlds.end(); iter++, index++)
		{
			allWorlds[index] = iter->second;
		}

		//Call creation of reduced record, get the wrapp everywhere function from the wrapper for that
		propagateCommsP fP;
		if (getWrapperFunction ("propagateComms", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
		{
			(*fP) (
                pId,
				minRank,
				maxRank,
				worldSize,
				commNull,
				commSelf,
				commWorld,
				allNumWorlds,
				allWorlds
				);
		}

		myWasSuccessful = true;
		delete []allWorlds;

		return GTI_ANALYSIS_SUCCESS;
	}

	//Waiting for more, add to list of blocked partners
	myReductionPartners.push_back (thisChannel);
	return GTI_ANALYSIS_WAITING;
}

//=============================
// timeout
//=============================
void CommReduction::timeout (void)
{
	if (!myTimedOut && !myWasSuccessful)
	{
	    //We do not rely on this anymore, we now use: ModuleBase::getReachableRanks
		//std::cerr << "Internal MUST error: the reduction on predefined communicators timed out, this should usually not happen and will have consequences on your run. Either some critical error happened or the system is starting up very slowly. As a result it may happen that some correctness checks won't be able to run until the missing data arrives. " << std::endl;
	}

	if (myReductionPartners.size() > 0)
	{
		//remove old partners
		std::list<I_ChannelId*>::iterator i;
		for (i = myReductionPartners.begin(); i != myReductionPartners.end(); i++)
		{
			delete (*i);
		}
		myReductionPartners.clear();
	    myTimedOut = true; //Only mark as timed out if we started at least
	}
}

/*EOF*/
