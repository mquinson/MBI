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
 * @file SumFloatReduction.cpp
 *       Implementation for the reduction in a reduction example.
 *
 *  @date 23.12.2010
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "SumFloatReduction.h"
#include "ModuleBase.h"
#include "CompletionTree.h"

namespace gti
{
	/**
     * Reduction class.
     */
    class SumFloatReduction : public ModuleBase<SumFloatReduction, I_SumFloatReduction>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		SumFloatReduction (const char* instanceName);

        /**
         * Performs an analysis.
         * @param f float number to sum up.
         * @param thisChannel channel of the new float value (See gti::I_Reduction)
         * @param outFinishedChannels list of channels finished (See gti::I_Reduction)
         * @return return value of reduction.
         */
    	    GTI_ANALYSIS_RETURN reduce (float f, I_ChannelId *thisChannel, std::list<I_ChannelId*> *outFinishedChannels);

    	    /**
    	     * The timeout function, see gti::I_Reduction.timeout
    	     */
    	    void timeout (void);

    protected:
    	    std::map<I_ChannelId*, float> myReductionPartners;
    	    CompletionTree *myCompletion;
    	    std::list<CompletionTree*> myTimedOutReds;

    	    /**
    	     * Returns the current or a new completion tree.
    	     * @param any id, used to determine size of root node.
    	     * @return completion tree.
    	     */
    	    CompletionTree* getCompletionTree (I_ChannelId *id);
    }; /*class SumFloatReduction*/
} /*namespace gti*/

using namespace gti;

mGET_INSTANCE_FUNCTION(SumFloatReduction)
mFREE_INSTANCE_FUNCTION(SumFloatReduction)
mPNMPI_REGISTRATIONPOINT_FUNCTION(SumFloatReduction)

//=============================
// SumFloatReduction
//=============================
SumFloatReduction::SumFloatReduction (const char* instanceName)
    : ModuleBase<SumFloatReduction, I_SumFloatReduction> (instanceName),
      myReductionPartners (),
      myCompletion (NULL)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();
    //Needs no sub-modules ...
}

//TODO destructor needed!

//=============================
// reduce
//=============================
GTI_ANALYSIS_RETURN SumFloatReduction::reduce (
		float f,
		I_ChannelId *thisChannel,
		std::list<I_ChannelId*> *outFinishedChannels)
{
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
	CompletionTree *tree = getCompletionTree(thisChannel);
	tree->addCompletion(thisChannel);

	if (tree->isCompleted())
	{
		float sum = f;

		//Reset completions tree
		tree->flushCompletions();

		//Sum up and add finished partners to out list
		std::map<I_ChannelId*, float>::iterator i;
		for (i = myReductionPartners.begin(); i != myReductionPartners.end(); i++)
		{
			//set as reopened channel
			outFinishedChannels->push_back (i->first);

			//build sum
			sum += i->second;
		}

		//Important: clear stored partners ...
		myReductionPartners.clear();

		//Call creation of reduced record, get the wrapp everywhere function from the wrapper for that
		int (*reducedFloatSum) (float);
		if (getWrapperFunction ("reducedFloatSum", (GTI_Fct_t*)&reducedFloatSum) == GTI_SUCCESS)
			(*reducedFloatSum) (sum);

		return GTI_ANALYSIS_SUCCESS;
	}

	//Waiting for more, add to list of blocked partners
	myReductionPartners.insert (std::make_pair(thisChannel, f));
	return GTI_ANALYSIS_WAITING;
}

//=============================
// timeout
//=============================
void SumFloatReduction::timeout (void)
{
	//Did we start a reduction at all (if not, nothing to do here)
	if (myReductionPartners.size() > 0)
	{
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
CompletionTree* SumFloatReduction::getCompletionTree (I_ChannelId *id)
{
	if (!myCompletion)
		myCompletion = new CompletionTree (id->getNumUsedSubIds()-1, id->getSubIdNumChannels(id->getNumUsedSubIds()-1));
	return myCompletion;
}

/*EOF*/
