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
 * @file FinalizeReduction
 *       @see MUST::FinalizeReduction.
 *
 *  @date 04.04.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "BaseApi.h"

#include "FinalizeReduction.h"

using namespace must;

mGET_INSTANCE_FUNCTION(FinalizeReduction)
mFREE_INSTANCE_FUNCTION(FinalizeReduction)
mPNMPI_REGISTRATIONPOINT_FUNCTION(FinalizeReduction)

//=============================
// Constructor
//=============================
FinalizeReduction::FinalizeReduction (const char* instanceName)
    : gti::ModuleBase<FinalizeReduction, I_FinalizeReduction> (instanceName),
      myReductionPartners (),
      myCompletion (NULL),
      myTimedOut (false)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    if (subModInstances.size() > 0)
    {
    		for (unsigned int i = 0; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }

    //Initialize module data
    //nothing to do
}

//=============================
// Destructor
//=============================
FinalizeReduction::~FinalizeReduction ()
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
GTI_ANALYSIS_RETURN FinalizeReduction::reduce (
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

		//Call creation of reduced record, get the wrapp everywhere function from the wrapper for that
		//Note: This is somewhat weird here, the notifyFinalize call has no arguments, its only indirect argument
		//          is the channel id that comes with the event, analyses will evaluate that. By replacing the record
		//          we create a new channel id for this event which is our intention.
		finalizeNotifyP fP;
		if (getWrapperFunction ("finalizeNotify", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
		{
			(*fP) ();
		}

		return GTI_ANALYSIS_SUCCESS;
	}

	//Waiting for more, add to list of blocked partners
	myReductionPartners.push_back (thisChannel);
	return GTI_ANALYSIS_WAITING;
}

//=============================
// timeout
//=============================
void FinalizeReduction::timeout (void)
{
	if (myReductionPartners.size() > 0)
	{
		//remove old partners
		std::list<I_ChannelId*>::iterator i;
		for (i = myReductionPartners.begin(); i != myReductionPartners.end(); i++)
		{
			delete (*i);
		}
		myReductionPartners.clear();

		myTimedOut = true;
	}
}

/*EOF*/
