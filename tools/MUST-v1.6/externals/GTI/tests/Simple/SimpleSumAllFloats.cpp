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
 * @file SimpleSumAllFloats.cpp
 *       A module for the simple weaver test case.
 *
 *  @date 25.08.2010
 *  @author Tobias Hilbrich
 */

#include <assert.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <iostream>
#include <fstream>

#include "I_Reduction.h"
#include "GtiMacros.h"
#include "SimpleMod.h"
#include "ModuleBase.h"
#include "GtiEnums.h"
#include "CompletionTree.h"

namespace gti
{
	/**
     * Check class.
     */
    class SumAllFloats : public ModuleBase<SumAllFloats, I_SumAllFloats>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    	SumAllFloats (const char* instanceName);

        /**
         * Performs an analysis.
         */
    	    GTI_ANALYSIS_RETURN analyse (float f, I_ChannelId *thisChannel, std::list<I_ChannelId*> *outFinishedChannels);

    	    /**
    	     * The timeout function, see gti::I_Reduction.timeout
    	     */
    	    void timeout (void);

    protected:
    	    std::map<I_ChannelId*, float> myReductionPartners;
    	    CompletionTree *myCompletion;
    	    std::list<CompletionTree*> myTimedOutReds;
    	    bool myTimedOut;

    	    CompletionTree* getCompletionTree (I_ChannelId *id);
    }; /*class SumAllFloats*/
} /*namespace gti*/

using namespace gti;

mGET_INSTANCE_FUNCTION(SumAllFloats)
mFREE_INSTANCE_FUNCTION(SumAllFloats)
mPNMPI_REGISTRATIONPOINT_FUNCTION(SumAllFloats)

//=============================
// SumAllFloats
//=============================
SumAllFloats::SumAllFloats (const char* instanceName)
    : ModuleBase<SumAllFloats, I_SumAllFloats> (instanceName),
      myReductionPartners (),
      myCompletion (NULL),
      myTimedOut (false)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Needs no sub-modules
    assert (subModInstances.empty());
}

//=============================
// analyse
//=============================
GTI_ANALYSIS_RETURN SumAllFloats::analyse (float f, I_ChannelId *thisChannel, std::list<I_ChannelId*> *outFinishedChannels)
{
	std::cout << "SumAllFloats (f=" << f << ") (num channs: " << thisChannel->getSubIdNumChannels(thisChannel->getNumUsedSubIds()-1) << ")" << std::endl;
	CompletionTree *tree = getCompletionTree(thisChannel);

	//1)====================================
	//Is this a record that was missing in a timed out reduction ?
	std::list<CompletionTree*>::iterator timedIter;
	for (timedIter = myTimedOutReds.begin(); timedIter != myTimedOutReds.end(); timedIter++)
	{
		CompletionTree * current = *timedIter;

		//Was it already completed in the tree ?
		if (current->wasCompleted(thisChannel))
			continue;

		//Was not completed in that tree, add completion
		current->addCompletion(thisChannel);

		//Is the reduction fininshed now ?
		if (current->isCompleted())
		{
			delete current;
			myTimedOutReds.erase (timedIter);
		}

		return GTI_ANALYSIS_IRREDUCIBLE;
	}

	//2)====================================
	//This is a record of an active and possibly successful reduction
	tree->addCompletion(thisChannel);

	////DEBUG
	//static std::ofstream outS;
	//static bool isOpen = false;
	//if (!isOpen)
	//{
	//	isOpen = true;
	//	char temp[256];
	//	sprintf(temp, "temp_%d.dot", getpid());
	//	outS.open(temp);
	//}
	//tree->printAsDot(outS);

	if (tree->isCompleted())
	{
		float mySum = f;

		//Reset completions tree
		tree->flushCompletions();

		//Sum up and add finished partners to out list
		std::map<I_ChannelId*, float>::iterator i;
		for (i = myReductionPartners.begin(); i != myReductionPartners.end(); i++)
		{
			//set as reopened channel
			outFinishedChannels->push_back (i->first);

			mySum += i->second;
		}

		//Important: clear stored partners ...
		myReductionPartners.clear();

		//Call creation of reduced record
		int (*reducedFloatSum) (float);
		if (getWrapperFunction ("reducedFloatSum", (GTI_Fct_t*)&reducedFloatSum) == GTI_SUCCESS)
		{
			(*reducedFloatSum) (mySum);
		}
		else
		{
			std::cout << "ERROR: failed to get \"reducedFloatSum\" function pointer from wrapper." << std::endl;
			assert (0);
		}

		return GTI_ANALYSIS_SUCCESS;
	}
	else
	{
		//Waiting for more, add to list of blocked partners
		myReductionPartners.insert (std::make_pair(thisChannel, f));
		return GTI_ANALYSIS_WAITING;
	}

    return GTI_ANALYSIS_FAILURE;
}

//=============================
// timeout
//=============================
void SumAllFloats::timeout (void)
{
	//Did we start a reduction at all (if we did not we didn't block any channels and need not act)
	if (! myReductionPartners.empty())
	{
		myTimedOut = true;

		//remove old partners
		std::map<I_ChannelId*, float>::iterator i;
		for (i = myReductionPartners.begin(); i != myReductionPartners.end(); i++)
			delete (i->first);
		myReductionPartners.clear();

		//move old completion tree to list of timed out reductions
		if (myCompletion)
			myTimedOutReds.push_back (myCompletion);
		myCompletion = NULL;
	}
}

//=============================
// getCompletionTree
//=============================
CompletionTree* SumAllFloats::getCompletionTree (I_ChannelId *id)
{
	if (!myCompletion)
		myCompletion = new CompletionTree (id->getNumUsedSubIds()-1, id->getSubIdNumChannels(id->getNumUsedSubIds()-1));
	return myCompletion;
}

/*EOF*/
