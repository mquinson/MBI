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
 * @file I_ChannelId.h
 *       @see gti::I_ChannelId
 *
 * @author Tobias Hilbrich
 * @date 6.12.2010
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#include <stdint.h>
#include <string>

#ifndef UINT32_MAX
#define UINT32_MAX  (0xffffffff)
#endif /*UINT32_MAX*/

#ifndef I_CHANNELID_H
#define I_CHANNELID_H

namespace gti
{
	/**
	 * This interface is used to access an abstract identifier for a process in the application layer.
	 * It consists of multiple sub-ids each of these sub-ids is a pair of the form (fromChannel, numChannels).
	 * The "fromChannel" represents from which channel of a tool place the record was
	 * received. E.g. 0.2.1 (read right to left, in first layer received from channel 1, in second layer received from
	 * channel 2, in third layer received from channel 0)
	 * So if a certain place receives a channel-id and writes the channel-id from which it received the
	 * record to the id, it tells the place which channels to use to ultimately arrive at the application
	 * process that created the record. The second element in the pair "numChannels" is used
	 * to determine how many connections are leading to the place where the record was received with
	 * "fromChannel". This piece of information is crucial for reductions in order to determine whether
	 * they received information from all partners that need to contribute to this reduction.
	 *
	 * This is encapsulated into this interface as the size of data needed for such an id may depend
	 * on the actual system configuration (number of levels, number of channels per place, ...).
	 *
	 * Sorting is done on a per sub-id basis (using the fromChannel value) starting at the leftmost
	 * sub-id (the one of highest order). E.g. 0.5 < 1.1; 0.2 < 0.3
	 */
	class I_ChannelId
	{
	public:

	    /**
	     * Destructor.
	     */
	    inline virtual ~I_ChannelId() {}

		/**
		 * Sets the "fromChannel" for the sub id for the given level to the given channel.
		 * @param level for which the sub-id is going to be set.
		 * @param channel to set the sub-id to.
		 */
		virtual void setSubId (int level, uint64_t channel) = 0;

		/**
		 * Returns the "fromChannel" for sub-id of the given level.
		 * Either the return value is a channel id, or it is -1
		 * to denote that no sub-id was set for this level.
		 * @param level for which to return the sub-id.
		 * @return sub-id for level (>=0), or -1 if not set.
		 */
		virtual long getSubId (int level) = 0;

		/**
		 * Sets the "numChannel" for the sub id for the given level to the given value.
		 * @param level for which the numChannels sub-id is going to be set.
		 * @param numChannels value to set.
		 */
		virtual void setSubIdNumChannels (int level, uint64_t numChannels) = 0;

		/**
		 * Returns the "numChannel" for sub-id of the given level.
		 * @param level for which to return the "numChannel" from the sub-id.
		 * @return number of channels for sub-id of given level.
		 */
		virtual long getSubIdNumChannels (int level) = 0;

		/**
		 * Compares two channel-ids.
		 * Assumes that the number of channels is same on all
		 * @param other channel-id to compare to.
		 * @return true iff equal.
		 */
		virtual bool isEqual (I_ChannelId* other) = 0;

		/**
		 * Determines whether this channel-id is less than the given
		 * channel-id is in terms of ordering.
		 * @param other channel id to test with.
		 * @return true iff this channel id is less than other.
		 */
		virtual bool isLessThan (I_ChannelId* other) = 0 ;

		/**
		 * Creates a copy of this channel-id.
		 * @return copy.
		 */
		virtual I_ChannelId* copy (void) = 0;

		/**
		 * Returns a string representation of the channel id.
		 * Uses the format mentioned in the class description,
		 * e.g.: 0.2.5.6.2 (read from right to left).
		 * @return string representation.
		 */
		virtual std::string toString (void) = 0;

		/**
		 * Returns the number of levels in the channel-id.
		 * @return number of levels.
		 */
		virtual int getNumLevels (void) = 0;

		/**
		 * Returns the number of sub ids that are used for this
		 * channel id. If return value is X, than sub-ids 0...X-1
		 * are used.
		 * @return number of used sub-ids.
		 */
		virtual int getNumUsedSubIds (void) = 0;

		/**
		 * Specifies that the channel identifier only includes a strided set
		 * of the rank range it describes.
		 *
		 * For a single rank in the range, multiple strides could be specified,
		 * thus use UINT32_MAX as stride to describe a single element.
		 *
		 * To remove stride representation specify a stride of 0.
		 *
		 * @param offset first rank in the channel identifiers range.
		 * @param stride of the representation.
		 */
		virtual void setStrideRepresentation (uint32_t offset, uint32_t stride) = 0;

		/**
		 * Returns true if this is a stride representation.
		 * If so provides offset and stride of the representation.
		 *
		 * if outStride is set to UINT32_MAX then the stride only include a single element
		 * (specified by the offset).
		 *
		 * @param outOffset pointer to storage for offset or NULL.
		 * @param outStride pointer to storage for stride or NULL.
		 * @return true if strided representation.
		 */
		virtual bool isStrideRepresentation (uint32_t *outOffset, uint32_t *outStride) = 0;
	};

	/**
	 * Small helper class to provide sorting in maps
	 * with pointers to I_ChannelId.
	 */
	class I_ChannelIdComp
	{
	public:
		/**
		 *  Compare function as it can be passed to std::map as third
		 *  template argument.
		 *  @param lhs channel id of left side of "<".
		 *  @param rhs channel id of right side of ">".
		 *  @return success of lhr < rhs.
		 */
		  bool operator() (I_ChannelId* lhs, I_ChannelId* rhs) const
		  {
			  return lhs->isLessThan(rhs);
		  }
	};
}

#endif //#ifdef I_CHANNELID_H
