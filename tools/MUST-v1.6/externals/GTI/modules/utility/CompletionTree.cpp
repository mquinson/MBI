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
 * @file CompletionTree.cpp
 *       @see gti::CompletionTree
 *
 * @author Tobias Hilbrich
 * @date 17.12.2010
 */

#include <assert.h>

#include "CompletionTree.h"

using namespace gti;

//=============================
// CompletionTree
//=============================
CompletionTree::CompletionTree (int subIdIndex, long numChannels)
 : I_ChannelTree<CompletionTree> (subIdIndex, numChannels),
   myIsCompleted (false),
   myNumChildsCompleted (0),
   myHasCompletedAncestor (false)
{
	/*Nothing to do*/
}

//=============================
// ~CompletionTree
//=============================
CompletionTree::~CompletionTree ()
{
	/*Nothing to do*/
}

//=============================
// flushCompletions
//=============================
void CompletionTree::flushCompletions (void)
{
	myIsCompleted = false;
	myNumChildsCompleted = 0;
	myHasCompletedAncestor = false;

	std::map<long int, CompletionTree*>::iterator iter;
	for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
	{
		iter->second->flushCompletions();
	}
}

//=============================
// isCompleted
//=============================
bool CompletionTree::isCompleted (void)
{
	if (myIsCompleted)
		return true;

	if (myNumChildsCompleted == myNumChannels && myNumChildsCompleted > 0)
		return true;

	return false;
}

//=============================
// addCompletion
//=============================
void CompletionTree::addCompletion (I_ChannelId* idCompleted)
{
	CompletionTree *node = getChildForChannel (idCompleted);

	assert (node != NULL);

	if (node == this)
	{
		myIsCompleted = true;
	}
	else
	{
		bool wasCompleted = node->isCompleted();

		node->addCompletion(idCompleted);

		bool isCompletedNow = node->isCompleted();

		if (!wasCompleted && isCompletedNow)
			myNumChildsCompleted++;

		myHasCompletedAncestor = true;
	}
}

//=============================
// getNodeColor
//=============================
std::string CompletionTree::getNodeColor (void)
{
	if (myIsCompleted)
		return "green";

	if (isCompleted())
		return "yellow";

	return "red";
}

//=============================
// wasCompleted
//=============================
bool CompletionTree::wasCompleted (I_ChannelId* id)
{
	long int subId = getChannelForId (id);

	//If this node is already completed, all sub-nodes for which the id might be will also be completed
	if (myIsCompleted)
		return true;

	//Is this the node for the id ? -> Completed if any ancestor completed!
	if (subId == -1)
		return myHasCompletedAncestor;

	//Does the child actually exists ?
	std::map<long int, CompletionTree*>::iterator pos = myChilds.find (subId);
	if (pos == myChilds.end())
		return false;

	//Recurse
	return pos->second->wasCompleted(id);
}

//=============================
// createsNewChild
//=============================
bool CompletionTree::createsNewChild (I_ChannelId* id)
{
    long int subId = getChannelForId (id);

    if (myChilds.find (subId) == myChilds.end())
        return true;
    return false;
}

//=============================
// copy
//=============================
CompletionTree* CompletionTree::copy (void)
{
    CompletionTree* ret =  new CompletionTree (mySubIdIndex, myNumChannels);

    ret->myIsCompleted = myIsCompleted;
    ret->myNumChildsCompleted = myNumChildsCompleted;
    ret->myHasCompletedAncestor = myHasCompletedAncestor;

    std::map<long int, CompletionTree*>::iterator iter;
    for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
    {
        ret->myChilds.insert(std::make_pair (iter->first, iter->second->copy()));
    }

    return ret;
}

/*EOF*/
