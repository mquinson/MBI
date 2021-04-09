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
 * @file SuspensionBufferTree.h
 *       @see gti::SuspensionBufferTree
 *
 * @author Tobias Hilbrich
 * @date 13.12.2009
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#include "I_ChannelId.h"
#include "I_ChannelTree.h"
#include <deque>
#include <map>
#include <string>
#include <list>
#include <ostream>

#ifndef SUSPENSION_BUFFER_TREE_H
#define SUSPENSION_BUFFER_TREE_H

namespace gti
{
	/**
	 * Storage for information on a received record and the
	 * channel id for it, which was created with a call to
	 * I_PlaceReceival::getUpdatedChannelId.
	 * Used by ChannelTree.
	 */
	class RecordInfo
	{
	public:
		void* buf;
		uint64_t num_bytes;
		void* free_data;
		GTI_RETURN (*buf_free_function) (void* free_data, uint64_t num_bytes, void* buf);
		I_ChannelId *recordChannelId;
	}; //RecordInfo

	/**
	 * Tree that reflects channel ids and stores information on the nodes.
	 * Used to store whether information of a certain channel id or a range of ids
	 * is currently blocked and which records had to be enqueued as a result of
	 * such a suspension.
	 */
	class SuspensionBufferTree : public I_ChannelTree<SuspensionBufferTree>
	{
	public:
		/**
		 * Creates a node of the channel id tree. For root node.
		 * @param subIdIndex index into the sub-id of the channel id that is used to branch on this node,
		 *               if -1 this is a leaf and no further branching is needed on this node.
		 * @param numChannels number of channels connected to this node.
		 */
		SuspensionBufferTree (int subIdIndex, long numChannels);

		/**
		 * Creates a node of the channel id tree. For child nodes
		 * @param subIdIndex index into the sub-id of the channel id that is used to branch on this node,
		 *               if -1 this is a leaf and no further branching is needed on this node.
		 * @param numChannels number of channels connected to this node.
		 * @param parent of this node or NULL.
		 */
		SuspensionBufferTree (int subIdIndex, long numChannels, SuspensionBufferTree* parent);

		/**
		 * Destructor.
		 */
		~SuspensionBufferTree (void);

		/**
		 * Returns the node that represents the given channel id in the tree.
		 * Will add the node if it does currently not exists.
		 * @param id that represents the node.
		 * @param outFirstSuspendedNode pointer to storage for a pointer to a tree node or null, if not null it must be preset to NULL and
		 *               will be set to the
         *               first suspended node that was on the path to the node for this channel id, only modified if such a node
         *               was encountered.
         * @param outFirstNonEmptyQueue first node along the path to the node for the channel id that had a non empty queue.
         * @return the node for the given id, NULL if an error occurred, e.g. given id has not enough sub ids.
		 */
		SuspensionBufferTree* getNode (I_ChannelId* id, SuspensionBufferTree **outFirstSuspendedNode, SuspensionBufferTree **outFirstNonEmptyQueue);

		/**
		 * Returns a queued record of any not suspended node or leaf.
		 * A nodes queued record may only be returned if none of its
		 * childs is suspended and none of its childs has any queued
		 * record. Will always return the front record of a suitable record.
		 *
		 * Uses popFront to rebalance records in case records may be
		 * redistributed to childs due to the removal of a record.
		 *
		 * @param outRecord pointer to storage for record info.
		 * @return true if a record was found, false otherwise.
		 */
		bool getQueuedRecord (RecordInfo *outRecord);

		/**
		 * Returns true if this node is suspended or all childs return
		 * true for this call. If some channels don't have childs yet
		 * they are assumed as unsuspended.
		 *
		 * @return true if tree starting from this node is completely suspended.
		 */
		bool isCompletlySuspended (void);

		/**
		 * Returns true if this node is suspended or any of its descendants is
		 * suspended.
		 * @return true if this or any descendant if suspended.
		 */
		bool hasAnySuspension (void);

		/**
		 * Returns true if this node is suspended.
		 * @return true or false.
		 */
		bool isSuspended (void);

		/**
		 * Sets whether this node is suspended or not.
		 * @param suspended true if this node shall be suspended, false otherwise.
		 */
		void setSuspension (bool suspended, I_ChannelId *reason);

		/**
		 * Removes all channel suspensions.
		 */
		void removeAllSuspensions (void);

		/**
		 * Returns the number of records in the queue for this node.
		 * @return size of queue.
		 */
		size_t getQueueSize(void);

		/**
		 * Returns the total number of queued events in all child nodes of this node.
		 * This does not include any events queued on this node.
		 * @return number of queued events in all childs.
		 */
		int getChildQueueSize(void);

		/**
		 * Returns the first queued record info and removes it from the queue.
		 * Outcome of this function is unspecified if no queued element exists.
		 *
		 * If this is a node, all successive records in queue will be put to childs or leafs
		 * until the queue is empty or its first element belongs to this node,
		 * i.e. its sub-id for this node is -1.
		 *
		 * @return information on first queued element.
		 */
		RecordInfo popFront (void);

		/**
		 * Adds information on a record to the end of the queue.
		 * @param info on record to enqueue.
		 */
		void pushBack(RecordInfo info);

		/**
		 * Returns a list of children indices of this node that are suspended
		 * themselves or that have a suspended child.
		 * @return list with indices to children.
		 */
		std::list<long> getChildsIndicesWithSuspension (void);

	protected:
		int mySuspensionCount; /**< True if this node is currently suspended. */
		std::deque<RecordInfo> myQRecords; /**< Queue with suspended records. */
		SuspensionBufferTree* myParent;
		int myStride;/*0 for any stride (single element), 1 for continuous, > 1 for a stride*/
		std::list<int> myOffsets; /*List of included offsets, only used if myStride != 1*/

		/*
		 * The following two variables are used to speed up some querries and must be keept
		 * consistent in the tree.
		 */
		int myNumChildRecords; /**< Recursive number of records queued in childs, excluding this node.*/
		int myNumSuspendedChilds; /**< Recursive number of suspensions in childs, excluding this node.*/

		/**
		 * Used to implement ChannelTree::getQueuedRecord.
		 * Has an extra argument that stores whether ther was any suspension in a sub-tree.
		 * @param outRecord as in getQueuedRecord.
		 * @param outHadASuspension extra argument to store whether this sub-tree had any suspension.
		 * @return outRecord as in getQueuedRecord.
		 */
		bool getQueuedRecordRecurse (RecordInfo *outRecord, bool *outHadASuspension);

		/**
		 * @see gti::I_ChannelTree::getNodeName
		 */
		virtual std::string getNodeName (void);

		/**
		 * @see gti::I_ChannelTree::getNodeColor
		 */
		virtual std::string getNodeColor (void);

		/**
		 * @see gti::I_ChannelTree::getNodeExtraRows
		 */
		virtual std::string getNodeExtraRows (void);

		/**
		 * Overwritten to pass parent node during node creation.
		 * @see gti::I_ChannelTree::allocateChild
		 */
		virtual SuspensionBufferTree* allocateChild (int subIdIndex, long numChannels);

		/**
		 * Called by childs for their parent to notify
		 * of a newly enqueued record, is forwarded recursively
		 * upwards in the tree.
		 */
		void incChildRecordNum (void);

		/**
		 * Called by childs for their parent to notify
		 * of a dequeued record, is forwarded recursively
		 * upwards in the tree.
		 */
		void decChildRecordNum (void);

		/**
		 * Called by childs for their parent to notify
		 * of a newly suspension, is forwarded recursively
		 * upwards in the tree.
		 */
		void incChildSuspensions (void);

		/**
		 * Called by childs for their parent to notify
		 * of a removed suspension, is forwarded recursively
		 * upwards in the tree.
		 */
		void decChildSuspensions (void);

		/**
		 * Helper to test whether the given channel identifier refers to this very node and
		 * whether it does not overlaps with our own stride representation, e.g, is compatible
		 * to it.
		 */
		bool sameNodeAndStrideCompatible (I_ChannelId *id);

	};//SuspensionBufferTree

} //namespace gti

#endif //SUSPENSION_BUFFER_TREE_H
