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
 * @file GtiChannelId.h
 *       @see gti::GtiChannelId
 *
 * @author Tobias Hilbrich
 * @date 7.12.2010
 */

#include <string>

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"

#include "I_ChannelId.h"

#ifndef GTICHANNELID_H
#define GTICHANNELID_H

namespace gti
{
	/**
	 * Implements I_ChannelId.
	 * @see I_ChannelId
	 *
	 * N64 - number of 64 bit values needed to store the complete channel id
	 * NLevels - number of levels in the channel id (number of sub-ids)
	 *                 Each sub-id will use 2xNBitsPerLevel !
	 * NBitsPerLevel - number of bits needed for one of the two values of a sub-id (They are both of equal size)
	 */
	template <int N64, int NLevels, int NBitsPerLevel>
	class GtiChannelId : public I_ChannelId
	{
	public:
		/**
		 * Constructor.
		 * @param numUsed number of sub-ids that are used for this channel id.
		 */
		GtiChannelId (int numUsed);

		/**
		 * Destructor.
		 */
		~GtiChannelId ();

		/**
		 * Sets the n-th 64bit value of the channel id to the
		 * given bits.
		 * @param n index of 64bit block to set.
		 * @param bits the value to set.
		 */
		void set64 (int n, uint64_t bits);

		/**
		 * Returns the n-th 64bit value of the channel id.
		 * @param n index of 64bit block to get.
		 * @return n-th 64bit value.
		 */
		uint64_t get64 (int n);

		/**
		 * @see I_ChannelId::setsubId
		 */
		void setSubId (int level, uint64_t channel);

		/**
		 * @see I_ChannelId::getSubId
		 */
		long getSubId (int level);

		/**
		 * @see I_ChannelId::setSubIdNumChannels
		 */
		void setSubIdNumChannels (int level, uint64_t numChannels);

		/**
		 * @see I_ChannelId::getSubIdNumChannels
		 */
		long getSubIdNumChannels (int level);

		/**
		 * @see I_ChannelId::isEqual
		 */
		bool isEqual (I_ChannelId* other);

		/**
		 * @see I_ChannelId::isLessThan
		 */
		bool isLessThan (I_ChannelId* other);

		/**
		 * @see I_ChannelId::copy
		 */
		I_ChannelId* copy (void);

		/**
		 * @see I_ChannelId::toString
		 */
		std::string toString (void);

		/**
		 * @see I_ChannelId::getNumLevels
		 */
		int getNumLevels (void);

		/**
		 * @see I_ChannelId::getNumUsedSubIds
		 */
		int getNumUsedSubIds (void);

		/**
		 * @see I_ChannelId::setStrideRepresentation
		 */
		void setStrideRepresentation (uint32_t offset, uint32_t stride);

		/**
		 * @see I_ChannelId::isStrideRepresentation
		 */
		bool isStrideRepresentation (uint32_t *outOffset, uint32_t *outStride);

	protected:
		uint64_t data[N64];
		int myNumUsed; /**< Number of sub-ids used in the channel info. */

		/**
		 * Internally used to set one value of a sub-id pair.
		 * @param index of the sub-id element to set.
		 * @value to set the indexed sub-id element to.
		 */
		void setValueIndexed (int index, uint64_t value);

		/**
		 * Internally used to return one value of a sub-id pair.
		 * @param index the sub-id element to set.
		 * @return the value set for this sub-id.
		 */
		long getValueIndexed (int index);

	}; // class GtiChannelId

#include "GtiChannelId.hpp"

} // namespace gti

#endif //#ifdef GTICHANNELID_H
