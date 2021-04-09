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
 * @file GroupTable.h
 *       @see MUST::GroupTable
 *
 *  @date 07.03.2011
 *  @author Tobias Hilbrich
 */

#include "I_GroupTable.h"
#include "GroupTrack.h"

#include <vector>

#ifndef GROUPTABLE_H
#define GROUPTABLE_H

using namespace gti;

namespace must
{
	/**
	 * Implementation of must::I_GroupTable.
	 */
	class GroupTable : public HandleInfoBase, public I_GroupTable
	{
	    friend class GroupTrack;
	public:
		/**
		 * Constructs a group.
		 *
		 * The given vector must not have duplicates!
		 *
		 * @param set ranks in the group and their respective MPI_COMM_WORLD ranks.
		 * @param track to notify when this groups reference count becomes 0.
		 */
		GroupTable (std::vector<int> set, GroupTrack *track);

		/**
		 * Constructs a group.
		 *
		 * Uses a interval representation where group rank 0 is beginRank,
		 * rank 1 is beginRank+1, ...
		 *
		 * @param beginRank begining of interval, rank in MPI_COMM_WORLD.
		 * @param endRank (>= beginRank) end of interval, rank in MPI_COMM_WORLD.
		 * @param track to notify when this groups reference count becomes 0.
		 */
		GroupTable (int beginRank, int endRank, GroupTrack *track);

		/**
		 * @see must::I_GroupTable::translate.
		 */
		bool translate (int rank, int * outWorldRank);

		/**
		 * @see must::I_GroupTable::containsWorldRank.
		 */
		bool containsWorldRank (int worldRank, int *outGroupRank);

		/**
		 * @see must::I_GroupTable::getSize.
		 */
		int getSize ();

		/**
		 * @see must::I_GroupTable::getMapping.
		 */
		const std::vector<int>& getMapping (void);

		/**
		 * @see must::I_GroupTable::isEqual.
		 */
		bool isEqual (std::vector<int> *set);

		/**
		 * @see must::I_GroupTable::isEqual.
		 */
		bool isEqual (int beginRank, int endRank);

		/**
		 * @see must::I_GroupTable::isEqual.
		 */
		bool isEqual (I_GroupTable* group);

		/**
		 * @see HandleInfoBase::getResourceName
		 */
		std::string getResourceName (void);

		/**
		 * @see HandleInfoBase::printInfo
		 */
		bool printInfo (
		        std::stringstream &out,
		        std::list<std::pair<MustParallelId,MustLocationId> > *pReferences);

	protected:

		//Representation 1:
		/*
		 * The more powerful, but also more expensive representation.
		 */
		std::vector<int> mySet; /**< Entry i is rank i in this group, its value is the associated rank in MPI_COMM_WORLD.*/
		std::map<int, int> myWorld2Rank; /**< Reverse mapping of world ranks to ranks in the group, only initialized if needed.*/

		//Representation 2:
		/*
		 * This representation is active if myBeginRank is not < 0, otherwise representation 1 is active.
		 *
		 * For interval groups with linear mapping group rank 0 -> intervalBegin, rank 1 -> intervalBegin+1, ...
		 * Helpful for MPI_COMM_WORLD and MPI_COMM_SELF groups.
		 */
		int myBeginRank;
		int myEndRank;

		GroupTrack *myTrack; /**< GroupTrack instance to notify when this groups reference count becomes 0.*/

		/**
		 * Destructor.
		 */
		~GroupTable (void);

		/**
		 * Overwrite of deleteThis function of HandleInfoBase
		 */
		void deleteThis (void);
	};
} /*namespace MUST*/

#endif /*GROUPTABLE_H*/
