/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file I_DCollectiveMatchReduction.h
 *       @see I_DCollectiveMatchReduction.
 *
 *  @date 25.04.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#ifndef I_COLLECTIVEMATCHREDUCTION_H
#define I_COLLECTIVEMATCHREDUCTION_H

#include "I_Reduction.h"
#include "I_DCollectiveMatch.h"
#include "I_DCollectiveListener.h"
#include "I_CollCommListener.h"

using namespace must;

/**
 * Interface for distributed collective matching.
 * Version for reduction running across the
 * TBON nodes.
 */
class I_DCollectiveMatchReduction : public I_DCollectiveMatch, public gti::I_Reduction
{
public:
	virtual ~I_DCollectiveMatchReduction() {};

	/**
	 * Tells the analysis module on this layer whether any ancestor layer
	 * reduction module has intra layer communication.
	 * This is important information in order to decide whether intra layer
	 * communication based type matching was already done or not.
	 *
	 * This module must rewrite the first event of this type it receives and
	 * should remove all others.
	 *
	 * @param hasIntra true if an ancestor layer had intra layer communication.
	 * @return @see gti::GTI_ANALYSIS_RETURN
	 */
	virtual gti::GTI_ANALYSIS_RETURN ancestorHasIntraComm (
	        int hasIntra,
	        gti::I_ChannelId *cId,
	        std::list<gti::I_ChannelId*> *outFinishedChannels = NULL
	) = 0;

	/**
	 * Invoked if a remote place on the same layer passes us a set of (type, count)
	 * pairs for typematching (type is always the same).
	 * @param pId of info origin.
	 * @param lId of info origin.
	 * @param commRId of origins communicator.
	 * @param typeRId type to check.
	 * @param numCounts number of counts used with the type.
	 * @param counts for checking.
	 * @param firstRank start rank for counts.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN handleIntraTypeMatchInfo (
	        MustParallelId pId,
	        MustLocationId lId,
	        MustRemoteIdType commRId,
	        MustRemoteIdType typeRId,
	        int numCounts,
	        int* counts,
	        int firstRank,
	        int collectiveNumber,
	        int collId
	    ) = 0;

	/**
	 * Invoked if a remote place on the same layer passes us a set of (type, count)
	 * pairs for typematching (with one distinct type per pair).
	 * @param pId of info origin.
	 * @param lId of info origin.
	 * @param commRId of origins communicator.
	 * @param numCountsAndTypes number of pairs.
     * @param typeRIds types for checking.
	 * @param counts for checking.
	 * @param firstRank start rank for counts.
	 * @return @see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN handleIntraTypeMatchInfoTypes (
	        MustParallelId pId,
	        MustLocationId lId,
	        MustRemoteIdType commRId,
	        int numCountsAndTypes,
	        MustRemoteIdType* typeRIds,
	        int* counts,
	        int firstRank,
	        int collectiveNumber,
	        int collId
	    ) = 0;

	/**
	 * Registers a collective operation listener for this module.
	 */
	virtual void registerListener (must::I_DCollectiveListener *listener) = 0;

	/**
	 * Registers a communicator in use listener for this module.
	 */
	virtual void registerCommListener (must::I_CollCommListener *listener) = 0;
};

#endif /*I_COLLECTIVEMATCHREDUCTION_H*/
