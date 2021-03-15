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
 * @file I_ChannelTree.h
 *       @see gti::I_ChannelTree
 *
 * @author Tobias Hilbrich
 * @date 17.12.2009
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#include "I_ChannelId.h"
#include <map>
#include <string>
#include <ostream>
#include <stdio.h>

#ifndef I_CHANNEL_TREE_H
#define I_CHANNEL_TREE_H

namespace gti
{
	/**
	 * Base interface for creating trees that reflect channel ids.
	 * The IMPL template is the class of the interface implementation
	 * and is used when creating child nodes or when storing them.
	 * (Avoids casting)
	 */
	template <class IMPL>
	class I_ChannelTree
	{
	public:
		/**
		 * Destructor.
		 */
		virtual ~I_ChannelTree (void);

		/**
		 * Prints the node and all its children in the DOT representation.
		 * If nodeNamePrefix is set to "" it is assumed to be the root node and
		 * the surrounding "digraph ..." structure will be added to the output.
		 * @param out stream to use.
		 * @param nodeNamePrefix prefix to add to the dot identifier of this node (used
		 *               to enforce unique names in recursion, each node ads his own
		 *               name to its children), if left empty this node is used as root node.
		 * @return the stream.
		 */
		std::ostream& printAsDot (std::ostream& out, std::string nodeNamePrefix="");

	protected:
		std::map<long int, IMPL*> myChilds; /**< Child nodes of this node. */
		int mySubIdIndex; /**< Index into sub id of channel ids, if -1 this is a leaf.*/
		long myNumChannels; /**< Number of channels for this sub-id index.*/

		/**
		 * Creates a node of the tree.
		 * @param subIdIndex index into the sub-id of the channel id that is used to branch on this node,
		 *               if -1 this is a leaf and no further branching is needed on this node.
		 * @param number of channels that connect to this node
		 */
		I_ChannelTree (int subIdIndex, long numChannels);

		/**
		 * Returns the "fromChannel" value of the sub-id represented by this
		 * node of the tree from the given channel id.
		 *
		 * Return is:
		 *  > 0: if there is actually a child
		 *  - 1: it this is already the right node
		 *  - 2: if the channel id is invalid for this node
		 *
		 * @param channelId id to return the channel id from.
		 */
		long int getChannelForId (I_ChannelId* channelId);

		/**
		 * Returns the child of this node for the given channel id.
		 * If the channel id specifies -1 for the channel it returns this
		 * node instead. Returns NULL if the channelId was invalid.
		 * Will create the child if it is not present yet.
		 * @param channel id to get the child for.
		 * @return the child.
		 */
		IMPL* getChildForChannel (I_ChannelId* channelId);

		/**
		 * Allocates a new node of this tree.
		 * The default implementation passes the subIdIndex and the given
		 * number of channels to the tree. If the implementation needs
		 * further arguments in the constructor, it can overwrite this function
		 * in order to pass these extra arguments.
		 */
		virtual IMPL* allocateChild (int subIdIndex, long numChannels);

		/**
		 * Returns the name of this node, used as textual label when printing as DOT.
		 */
		virtual std::string getNodeName (void);

		/**
		 * Returns the DOT color to use for this node.
		 */
		virtual std::string getNodeColor (void);

		/**
		 * Each node is printed as a record in the DOT
		 * representation, using the form label="{{<NodeName>}|<ExtraRows>}"
		 * The <ExtraRows> content along with its prefixed | is added if this string
		 * returns a non empty string, the returned string is used as <ExtraRows>.
		 * @return string to add rows to DOT node.
		 */
		virtual std::string getNodeExtraRows (void);
	};//I_ChannelTree

#include "I_ChannelTree.hpp"

} //namespace gti

#endif //I_CHANNEL_TREE_H
