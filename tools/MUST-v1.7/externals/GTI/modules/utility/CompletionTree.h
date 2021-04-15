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
 * @file CompletionTree.h
 *       @see gti::CompletionTree
 *
 * @author Tobias Hilbrich
 * @date 17.12.2010
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#include "I_ChannelId.h"
#include "I_ChannelTree.h"

#ifndef COMPLETIONTREE_H
#define COMPLETIONTREE_H

namespace gti
{
	/**
	 * A tree that determines whether all processes have commited
	 * something to a reduction.
	 *
	 * @todo this should at a later time be extended to support setting a filter,
	 *            i.e. if a reduction for a collective on some comm only needs reductions
	 *            from some processes.
	 */
	class CompletionTree : public I_ChannelTree<CompletionTree>
	{
	public:
		/**
		 * Constructor.
		 * @param subIdIndex index into the sub-id of the channel id that is used to branch on this node,
		 *               if -1 this is a leaf and no further branching is needed on this node.
		 * @param numChannels number of channels leading to this node.
		 */
		CompletionTree (int subIdIndex, long numChannels);

		/**
		 * Destructor.
		 */
		~CompletionTree ();

		/**
		 * Removes all completions from the tree.
		 */
		void flushCompletions (void);

		/**
		 * Returns true iff this tree is completly completed.
		 *
		 * A node is completed iff:
		 *  - its completion state was set to true
		 *  - ALL its children (including ones that are potentially not in the tree yet) are considered completed according to this definition
		 *
		 *  @return true or false.
		 */
		bool isCompleted (void);

		/**
		 * Returns true iff this channel id OR parts of it were already completed.
		 * This is useful to determine whether a channel id that comes after
		 * a timeout belongs to the timed out reduction or to a new one.
		 * @param id to test for completion.
		 * @return true if given id was already completed, false otherwise.
		 */
		bool wasCompleted (I_ChannelId* id);

		/**
		 * Sets the node representing the given Id as completed.
		 * Also updates the number of childs completed for ancestors of the
		 * updated node, which speeds up the isCompleted call.
		 * @param idCompleted channel-id of a completed tree node.
		 */
		void addCompletion (I_ChannelId* idCompleted);

		/**
		 * Returns true if adding this channel id to this node would create a new child
		 * OF THIS NODE (i.e. a child not a descendant on a child of this node).
		 * False otherwise, i.e., this node already has the child that this id would require.
		 * @param id to check.
		 * @return see description.
		 */
		bool createsNewChild (I_ChannelId* id);

		/**
		 * Creates a copy of this tree.
		 * @return copy.
		 */
		CompletionTree* copy (void);

	protected:

		bool myIsCompleted; /**< Set to true if a channel id for this node was completed.*/
		long myNumChildsCompleted; /**< Used to determine whether this node is completed due to all childs being completed.*/
		bool myHasCompletedAncestor; /**< True if some ancestor is completed.*/

		/**
		 * @see gti::I_ChannelTree::getNodeColor
		 */
		virtual std::string getNodeColor (void);
	};
} //namespace gti

#endif // #ifndef COMPLETIONTREE_H
