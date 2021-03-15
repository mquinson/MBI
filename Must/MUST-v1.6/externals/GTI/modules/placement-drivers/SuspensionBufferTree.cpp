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
 * @file SuspensionBufferTree.cpp
 *       @see gti::SuspensionBufferTree
 *
 * @author Tobias Hilbrich
 * @date 13.12.2009
 */

#include <assert.h>
#include <iostream>
#include <sstream>

#include "SuspensionBufferTree.h"

using namespace gti;

//=============================
// SuspensionBufferTree
//=============================
SuspensionBufferTree::SuspensionBufferTree (int subIdIndex, long numChannels)
 : I_ChannelTree<SuspensionBufferTree> (subIdIndex, numChannels),
   mySuspensionCount (0),
   myQRecords (),
   myParent (NULL),
   myNumChildRecords (0),
   myNumSuspendedChilds (0),
   myStride (1),
   myOffsets ()
{
	/*Nothing to do*/
}

//=============================
// SuspensionBufferTree
//=============================
SuspensionBufferTree::SuspensionBufferTree (int subIdIndex, long numChannels, SuspensionBufferTree* parent)
 : I_ChannelTree<SuspensionBufferTree> (subIdIndex, numChannels),
   mySuspensionCount (0),
   myQRecords (),
   myParent (parent),
   myNumChildRecords (0),
   myNumSuspendedChilds (0),
   myStride (1),
   myOffsets ()
{
	/*Nothing to do*/
}

//=============================
// ~SuspensionBufferTree
//=============================
SuspensionBufferTree::~SuspensionBufferTree (void)
{
	//Free all enqueued records
	while (!myQRecords.empty())
	{
		RecordInfo info = myQRecords.front();
		myQRecords.pop_front();

		info.buf_free_function (info.free_data, info.num_bytes, info.buf);
	}

	//Do not delete the parent
	myParent = NULL;
}

//=============================
// getNode
//=============================
SuspensionBufferTree* SuspensionBufferTree::getNode (I_ChannelId* id, SuspensionBufferTree **outFirstSuspendedNode, SuspensionBufferTree **outFirstNonEmptyQueue)
{
    //Get node for this channel id
    SuspensionBufferTree *ret = getChildForChannel (id);
    bool wasCompatible = true;

	//Is this the first suspended node along the way ?
	if (mySuspensionCount && outFirstSuspendedNode)
	{
	    if (!sameNodeAndStrideCompatible(id))
	    {
	        if (*outFirstSuspendedNode == NULL)
	            *outFirstSuspendedNode = this;
	        wasCompatible = false;
	    }
	}

	//Is this the first node with a non-empty record queue along the way ?
	if (!myQRecords.empty() && outFirstNonEmptyQueue && *outFirstNonEmptyQueue == NULL)
	{
	    /**
	     * We can overtake the queue iff:
	     *  - new id is strided
	     *  - id is for this very node
	     *  - id is compatible with this node (wasCompatible == true)
	     *  - id is compatible with all existing queue items (simplified check for that)
	     */
	    bool overtake = false;
	    uint32_t offsetA, strideA;
	    bool isStrideA = id->isStrideRepresentation(&offsetA, &strideA);
	    if (ret == this && wasCompatible && isStrideA)
	    {
	        std::deque<RecordInfo>::iterator iter;
	        for (iter = myQRecords.begin(); iter != myQRecords.end(); iter++)
	        {
	            SuspensionBufferTree *temp = getChildForChannel (iter->recordChannelId);
	            if (temp != this)
	                break;

	            uint32_t offsetB, strideB;
	            bool isStrideB = iter->recordChannelId->isStrideRepresentation(&offsetB, &strideB);

	            if (    !isStrideB ||
	                    strideA != strideB ||
	                    strideA == UINT32_MAX ||
	                    offsetA % strideA == offsetB % strideA)
	                break;
	        }

	        if (iter == myQRecords.end())
	            overtake = true;
	    }

	    if (!overtake)
	        *outFirstNonEmptyQueue = this;
	}

	//was it invalid or is this the right node for the channel id ?
	if (ret == NULL || ret == this)
		return this;

	//recurse
	return ret->getNode(id, outFirstSuspendedNode, outFirstNonEmptyQueue);
}

//=============================
// isSuspended
//=============================
bool SuspensionBufferTree::isSuspended (void)
{
	return mySuspensionCount > 0;
}

//=============================
// setSuspension
//=============================
void SuspensionBufferTree::setSuspension (bool suspended, I_ChannelId *reason)
{
    uint32_t offset, stride;

	if (!suspended && mySuspensionCount == 1)
		if (myParent) myParent->decChildSuspensions();
	if (suspended && mySuspensionCount == 0)
		if (myParent) myParent->incChildSuspensions();

	if (suspended)
	{
	    mySuspensionCount++;

	    if (mySuspensionCount == 1)
	    {
	        //If this is the first suspension, it sets stride
	        if (reason->isStrideRepresentation(&offset, &stride))
	        {
	            if (stride == UINT32_MAX)
	                myStride = 0;
	            else
	                myStride = stride;

	            assert (myOffsets.empty());
	            myOffsets.push_back(offset);
	        }
	        else
	        {
	            myStride = 1;
	        }
	    }
	    else
	    {
	        //Add to reasons
	        assert (myStride != 1);//We can't be continous
	        bool ret = reason->isStrideRepresentation(&offset, &stride);
	        assert (ret); //The other must be in stride representation, otherwise it would overlap

	        if (myStride == 0 && stride == UINT32_MAX)
	        {
	            assert (!myOffsets.empty());
	            assert (offset != myOffsets.front());

	            if (offset > myOffsets.front())
	                myStride = offset - myOffsets.front();
	            else
	                myStride = myOffsets.front() - offset;
	        }
	        if (myStride == 0)
	        {
	            assert (!myOffsets.empty() && offset%stride != myOffsets.front()%stride); //We must have exactly one offset, and it must not be included in the new stride!
	            myStride = stride;
	            myOffsets.push_back(offset);
	        }
	        else if (myStride == stride)
	        {
	            std::list<int>::iterator iter;
	            for (iter = myOffsets.begin(); iter != myOffsets.end(); iter++)
	            {
	                assert ((*iter) % myStride != offset % stride); //We must not already have this particular offset in here!
	            }
	            myOffsets.push_back(offset);
	        }
	        else
	        {
	            //We should never have entered here!
	            //Overlap test should have said "no"
	            assert(0);
	        }

	    }
	}
	else
	{
	    mySuspensionCount--;

	    if (myStride == 1 || myStride == 0)
	    {
	        mySuspensionCount = 0;
	        myOffsets.clear ();
	        myStride = 0;
	    }
	    else
	    {
	        bool ret = reason->isStrideRepresentation(&offset, &stride);
	        assert (ret); //The other must be in stride representation, otherwise it would overlap
	        assert (stride == myStride || stride == UINT32_MAX);

	        std::list<int>::iterator iter;
	        bool found = false;
	        for (iter = myOffsets.begin(); iter != myOffsets.end(); iter++)
	        {
	            if (*iter%myStride == offset%myStride)
	            {
	                myOffsets.erase(iter);
	                found = true;
	                break;
	            }
	        }
	        assert (found); //We must have found that offset!

	        if (myOffsets.empty())
	        {
	            myStride = 0;
	        }
	    }
	}
}

//=============================
// getQueueSize
//=============================
size_t SuspensionBufferTree::getQueueSize(void)
{
	return myQRecords.size();
}

//=============================
// popFront
//=============================
RecordInfo SuspensionBufferTree::popFront (void)
{
	assert (!myQRecords.empty());
	RecordInfo ret = myQRecords.front();
	myQRecords.pop_front();
	if (myParent) myParent->decChildRecordNum();

	//If this is not a leaf: Push all records that are enqueued and do not belong to this node,
	//to the respective childs until the record queue is empty or the front element
	//belongs to this node (e.g. has a -1 as sub-id)
	while (!myQRecords.empty() && mySubIdIndex >= 0)
	{
		RecordInfo info = myQRecords.front();

		long int subId = getChannelForId(info.recordChannelId);

		//If first element is for this node: abort the redistribution loop
		if (subId == -1)
			break;

		//This element can be redistributed
		myQRecords.pop_front();
		/**
		 * @todo This is somewhat inefficent, the decrementing and incrementing
		 * resulting from this is absolutely unnecessary.
		 * A simple (but somewhat dirty) solution would be setting myParent
		 * to NULL and storing its old value. Afterwards do the requeuing and
		 * set myParent back to the old value afterward.
		 */
		if (myParent) myParent->decChildRecordNum();

		SuspensionBufferTree *target, *firstNonEmptyQ = NULL;
		target = getChildForChannel(info.recordChannelId)->getNode (info.recordChannelId, NULL, &firstNonEmptyQ);

		if (firstNonEmptyQ)
			target = firstNonEmptyQ;

		target->pushBack(info);
	}

	return ret;
}

//=============================
// pushBack
//=============================
void SuspensionBufferTree::pushBack(RecordInfo info)
{
	if (myParent) myParent->incChildRecordNum();
	myQRecords.push_back (info);
}

//=============================
// getQueuedRecord
//=============================
bool SuspensionBufferTree::getQueuedRecord (RecordInfo *outRecord)
{
	return getQueuedRecordRecurse (outRecord, NULL);
}

//=============================
// getQueuedRecord
//=============================
bool SuspensionBufferTree::getQueuedRecordRecurse (RecordInfo *outRecord, bool *outHadASuspension)
{
	bool noSuspensionInChilds = true;

	//Is this node suspended, if so no suitable record present if any child is suspended or has a non-empty queue
	//If the node is suspended and no child is suspended nor non-empty, then we may still succeed!
	if (mySuspensionCount && (myNumChildRecords > 0 || myNumSuspendedChilds > 0))
	{
		if (outHadASuspension)
			*outHadASuspension = true;
		return false;
	}

	//Is there anything at all in this sub-tree ? If not don't recurse further!
	if (myQRecords.empty() && myNumChildRecords == 0)
	{
		//There is some suspended child if myNumSuspendedChilds is > 0
		if (outHadASuspension)
			*outHadASuspension = (mySuspensionCount > 0 || myNumSuspendedChilds > 0); //mySuspensionCount is already handled above, so no need to put this into the calculation
		return false;
	}

	//Look in the child nodes for a suitable record
	//Their queued records have priority over this nodes records!
	std::map<long int, SuspensionBufferTree*>::iterator iter;
	for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
	{
		SuspensionBufferTree *child = iter->second;
		bool hadASuspension = false;

		if (child->getQueuedRecordRecurse(outRecord, &hadASuspension))
			return true;

		if (hadASuspension)
			noSuspensionInChilds = false;
	}

	//Set outHadASuspension correctly
	if (outHadASuspension)
		*outHadASuspension = !noSuspensionInChilds;

	//If no suspension in childs, we may use queued records of this node
	if (noSuspensionInChilds && !myQRecords.empty())
	{
	    if (mySuspensionCount)
	    {
	        //If we are suspended we may still have compatible events queued
	        //First of all we are very restrictive towards our childs and that we actually have a stride
	        if (myNumChildRecords > 0 || myNumSuspendedChilds > 0 || myStride == 1 || myStride == 0)
	        {
                if (outHadASuspension)
                    *outHadASuspension = true;
	            return false;
	        }

	        //Maybe the first queued event does the job
	        if (sameNodeAndStrideCompatible(myQRecords.front().recordChannelId))
	        {
	            if (outRecord)
	                *outRecord = popFront();
	            return true;
	        }

	        //Finally, go over out queued events till we find a compatible one
	        std::deque<RecordInfo>::iterator iter;
	        for (iter = myQRecords.begin(); iter != myQRecords.end(); iter++)
	        {
	            SuspensionBufferTree *temp = getChildForChannel (iter->recordChannelId);
	            if (temp != this)
	                break;

	            uint32_t offsetB, strideB;
	            bool isStrideB = iter->recordChannelId->isStrideRepresentation(&offsetB, &strideB);

	            if (    !isStrideB ||
	                    myStride != strideB ||
	                    myStride == 0)
	                break;

	            std::list<int>::iterator oIter;
	            for (oIter = myOffsets.begin(); oIter != myOffsets.end(); oIter++)
	            {
	                if ((*oIter) % myStride == offsetB % myStride)
	                    break;
	            }

	            if (oIter == myOffsets.end())
	            {
	                //If we arrive here we have only strided (and compatible) events in our queue and this
	                //is the first event that has an open offset, that event we dequeu
	                if (outRecord)
	                {
	                    *outRecord = *iter;
	                    if (myParent) myParent->decChildRecordNum();
	                    myQRecords.erase(iter);
	                }
	                return true;
	            }
	        }

	        if (outHadASuspension)
	            *outHadASuspension = true;
	        return false;
	    }
	    else
	    {
	        //If we are not suspended the first event is the right one :)
	        if (outRecord)
	            *outRecord = popFront();

	        return true;
	    }
	}

	return false;
}

//=============================
// isCompletlySuspended
//=============================
bool SuspensionBufferTree::isCompletlySuspended (void)
{
    std::map<long int, SuspensionBufferTree*>::iterator iter;

    if (mySuspensionCount)
    {
        if (myStride == 1)
            return true;

        if (myStride == 0 && myNumChannels == 1)
            return true;

        if (myStride > 1 && myOffsets.size() == myStride)
            return true;

        if (myNumChildRecords > 0 || myNumSuspendedChilds > 0)
            return true;
    }

    //If we have something in our queues, then the answer depends on whether the queued items are compatible!
    if (mySuspensionCount && !myQRecords.empty())
    {
        /*
         * This node is done for if:
         * - myStride == 1
         * - myStride == 0 && myNumChannels == 1
         * - myStride > 1 && mySuspensionCount == myStride
         * - If our queue poses an item that screws us
         * -- An item with stride 0 ||�1 (0 might work, but we are a bit restrictive after all :))
         * -- An item of another stride than our own one
         */
        if (myStride == 1)
            return true;
        if (myStride == 0 && myNumChannels == 1)
            return true;
        if (myStride > 1 && mySuspensionCount == myStride) //If so all potential offsets are used up!
            return true;

        std::deque<RecordInfo>::iterator iter;
        for (iter = myQRecords.begin(); iter != myQRecords.end(); iter++)
        {
            SuspensionBufferTree *temp = getChildForChannel (iter->recordChannelId);
            if (temp != this)
                return true;

            uint32_t offsetB, strideB;
            bool isStrideB = iter->recordChannelId->isStrideRepresentation(&offsetB, &strideB);

            if (    !isStrideB ||
                    myStride != strideB ||
                    myStride == 0)
                return true;

            //Check whether this event could be dequeued, if so we have something to process already and can abort this loop!
            std::list<int>::iterator oIter;
            for (oIter = myOffsets.begin(); oIter != myOffsets.end(); oIter++)
            {
                if ((*oIter) % myStride == offsetB % myStride)
                    break;
            }

            if (oIter == myOffsets.end())
            {
                //We have a queued event that can be processed, we can abort this loop!
                break;
            }
        }
    }

	/*
	 * Also consider: if we have something in the queue, while any child is suspended:
	 * Then no new events may be processed for this sub-tree
	 * However: there may still be queued events that are open for processing!
	 *
	 * With that our decision becomes:
	 * - isCompletelySuspended=true iff no event to dequeue in childs
	 * - isCompletelySuspended=false iff some event may still be dequeued
	 */
	if (myNumSuspendedChilds > 0 && !myQRecords.empty())
	{
	    if (mySuspensionCount == 0)
	    {
            for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
            {
                SuspensionBufferTree *child = iter->second;

                if (child->getQueuedRecordRecurse(NULL, NULL)) //Use of NULL for storage to return record ensured that we do not actually dequeue anything!
                    return false;
            }
	    }
	    return true;
	}

	//not all sub ids present ? (if so, not completely suspended !)
	if (myNumChannels >= 0 && myChilds.size() != myNumChannels)
		return false;

	//Is there anything at all in this sub-tree ? If not don't recurse further!
	if (myNumSuspendedChilds == 0)
	    return false;

	for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
	{
		SuspensionBufferTree *child = iter->second;

		if (!child->isCompletlySuspended())
			return false;
	}

	return true;
}

//=============================
// hasAnySuspension
//=============================
bool SuspensionBufferTree::hasAnySuspension (void)
{
	if (mySuspensionCount)
		return true;

	if (myNumSuspendedChilds > 0)
		return true;

	return false;
}

//=============================
// getChildQueueSize
//=============================
int SuspensionBufferTree::getChildQueueSize(void)
{
    return myNumChildRecords;
}

//=============================
// getNodeName
//=============================
std::string SuspensionBufferTree::getNodeName (void)
{
	char temp[128];
	sprintf (temp, "%d", mySubIdIndex);
	return temp;
}

//=============================
// getNodeColor
//=============================
std::string SuspensionBufferTree::getNodeColor (void)
{
	//Is suspended itself
	if (mySuspensionCount)
		return "red";

	//Is indirectly suspended, due to some child being suspended
	if (myNumSuspendedChilds > 0)
		return "yellow";

	//Is open for processing
	return "green";
}

//=============================
// getNodeExtraRows
//=============================
std::string SuspensionBufferTree::getNodeExtraRows (void)
{
	std::stringstream stream;
	stream
		<< "NumRecordsInChilds: " << myNumChildRecords << "|"
		<< "NumSuspendedChilds: " << myNumSuspendedChilds	 << "|"
		<< "myStride: " << myStride << "|"
		<< "myOffsets: ";

	std::list<int>::iterator iter;
	for (iter = myOffsets.begin(); iter != myOffsets.end(); iter++)
	{
	    if (iter != myOffsets.begin()) stream << ", ";
	    stream << *iter;
	}

	std::deque<RecordInfo> temp = myQRecords;
	while (!temp.empty())
	{
		RecordInfo info = temp.front();
		temp.pop_front();
		uint32_t offset=0, stride=1;
		bool isStrided = info.recordChannelId->isStrideRepresentation(&offset, &stride);

		stream << "|" << info.recordChannelId->toString() << ": " << ((uint64_t*)info.buf)[0] << " S:" << stride << " O:" << offset;
	}

	return stream.str();
}

//=============================
// removeAllSuspensions
//=============================
void SuspensionBufferTree::removeAllSuspensions (void)
{
	mySuspensionCount = 0;
	myOffsets.clear();
	myStride = 0;
	myNumSuspendedChilds = 0;

	std::map<long int, SuspensionBufferTree*>::iterator iter;
	for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
	{
		SuspensionBufferTree *child = iter->second;

		child->removeAllSuspensions();
	}
}

//=============================
// allocateChild
//=============================
SuspensionBufferTree* SuspensionBufferTree::allocateChild (int subIdIndex, long numChannels)
{
	return new SuspensionBufferTree (subIdIndex, numChannels, this);
}

//=============================
// incChildRecordNum
//=============================
void SuspensionBufferTree::incChildRecordNum (void)
{
	myNumChildRecords++;
	if (myParent) myParent->incChildRecordNum();
}

//=============================
// decChildRecordNum
//=============================
void SuspensionBufferTree::decChildRecordNum (void)
{
	myNumChildRecords--;
	assert (myNumChildRecords >= 0);
	if (myParent) myParent->decChildRecordNum();
}

//=============================
// incChildSuspensions
//=============================
void SuspensionBufferTree::incChildSuspensions (void)
{
	myNumSuspendedChilds++;
	if (myParent) myParent->incChildSuspensions();
}

//=============================
// decChildSuspensions
//=============================
void SuspensionBufferTree::decChildSuspensions (void)
{
	myNumSuspendedChilds--;
	assert (myNumSuspendedChilds >= 0);
	if (myParent) myParent->decChildSuspensions();
}

//=============================
// getChildsIndicesWithSuspension
//=============================
std::list<long> SuspensionBufferTree::getChildsIndicesWithSuspension (void)
{
    std::list<long> ret;

    std::map<long int, SuspensionBufferTree*>::iterator iter;
    for (iter = myChilds.begin(); iter != myChilds.end(); iter++)
    {
        if (!iter->second) continue;

        if (iter->second->hasAnySuspension ())
            ret.push_back(iter->first);
    }

    return ret;
}

//=============================
// sameNodeAndStrideCompatible
//=============================
bool SuspensionBufferTree::sameNodeAndStrideCompatible (I_ChannelId *id)
{
    bool accept = false; //Return value

    //Get node for this channel id
    SuspensionBufferTree *ret = getChildForChannel (id);

    //If the channel id is for this very node and it is the first suspended node, then we make a special
    //check to see whether we really overlap, i.e., use the stride representation of the channel id (if present)
    if (ret == this)
    {
        if (mySuspensionCount == 0)
            return true;

        uint32_t offset, stride;
        bool isStride = id->isStrideRepresentation(&offset, &stride);

        if (isStride)
        {
            if (myStride == 0 && stride == UINT32_MAX)
            {
                assert (!myOffsets.empty());
                if (offset != myOffsets.front())
                {
                    accept = true;
                }
            }
            if (myStride == 0)
            {
                if (offset%stride != myOffsets.front()%stride)
                {
                    accept = true;
                }
            }
            else if (myStride == stride)
            {
                accept = true;
                std::list<int>::iterator iter;
                for (iter = myOffsets.begin(); iter != myOffsets.end(); iter++)
                {
                    if ((*iter) % myStride == offset % stride)
                        accept = false;
                }
            }
        }
    }

    return accept;
}

/*EOF*/
