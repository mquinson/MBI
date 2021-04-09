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
 * @file I_Reduction.h
 *       @see gti::I_Reduction
 *
 * @author Tobias Hilbrich
 * @date 19.12.2010
 */

#ifndef I_REDUCTION_H
#define I_REDUCTION_H

#include "I_ChannelId.h"

namespace gti
{
	/**
	 * Interface that needs to be implemented by all redcutions.
	 *
	 * Furthermore, all of their analysis functions need add two extra arguments at the end
	 * of their argument lists:
	 * - I_ChannelId *thisChannel : channel id of the new record
	 *              Usage: if the analysis returns WAITING it will not be freed by the caller and must be
	 *              either freed by the reduction, or by adding it to the outFinishedChannels list at a successive call. For
	 *              all other return values the id will be deleted when the analysis call returns.
     * - std::list<I_ChannelId*> *outFinishedChannels : if the reduction returns SUCCESS or
     *              IRREDUCIBLE, all OTHER channel ids that were reduced as well need to be added
     *              to this list. The channel ids added to the list will be freed by the caller of the
     *              analysis function and must not be used by the reduction after the analysis call ended.
	 */
	class I_Reduction
	{
	public:
		/**
		 * Virtual destructor to make compilers happy.
		 */
		inline virtual ~I_Reduction (void) {};

		/**
		 * Notifies the reduction that any possibly ongoing reduction was aborted and possibly
		 * blocked channels were resumed. As a result, an analysis should still try to maintain its
		 * semantic behavior and count when all records for the reduction arrived. But it must
		 * return IRREDUCIBLE for these missing records. After the aborted reduction was complete
		 * (i.e. all its records arrived), a new reduction may be started and the WAITING/SUCCESS
		 * state may be returned again.
		 *
		 * Any channelIDs for which the reduction returned a WAITING without opening them up again
		 * on a SUCCESS return have not been forwarded. The memory of these channel ids must
		 * be freed by the reduction, but these ids mut not be returned as completed by the reduction!
		 */
		virtual void timeout (void) = 0;
	}; //I_Reduction
}//namespace gti

#endif //#ifdef I_REDUCTION_H
