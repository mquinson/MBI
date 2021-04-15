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
 * @file I_GroupTable.h
 *       @see must::I_GroupTable
 *
 *  @date 07.03.2011
 *  @author Tobias Hilbrich
 */

#ifndef I_GROUPTABLE_H
#define I_GROUPTABLE_H

#include "I_Destructable.h"

#include <vector>

namespace must
{
	/**
	 * Interface for accessing a group as defined in MPI.
	 * So it is a ordered set of process identifiers, as
	 * we have no information of these identifiers we use
	 * ranks in MPI_COMM_WORLD here.
	 */
	class I_GroupTable : public virtual I_Destructable
	{
	public:
		/**
		 * Translates the given rank into its corresponding rank in MPI_COMM_WORLD.
		 *
		 * @param rank to translate.
		 * @param outWorldRank corresponding rank in MPI_COMM_WORLD.
		 * @return true if the given rank is in the group, false otherwise
		 *              (outWorldRank will not be touched in that case).
		 */
		virtual bool translate (int rank, int * outWorldRank) = 0;

		/**
		 * Tests whether the given rank in MPI_COMM_WORLD is in this group
		 * and if so returns the rank within the group.
		 * @param worldRank to search.
		 * @param outGroupRank rank within this group of the searched for world rank, only set if found.
		 * @return true iff found.
		 */
		virtual bool containsWorldRank (int worldRank, int *outGroupRank) = 0;

		/**
		 * Returns the size of this group.
		 *
		 * @return size of this group.
		 */
		virtual int getSize () = 0;

		/**
		 * Returns a const reference to the mapping of the communicator.
		 * (Mapping of comm ranks to world ranks)
		 * @return mapping.
		 */
		virtual const std::vector<int>& getMapping (void) = 0;

		/**
		 * Returns true if the given group is equal to this group.
		 *
		 * The given set must not have duplicates !
		 *
		 * @param set of the group to compare against.
		 * @return true if both groups are equal (processes and
		 *              their order matches), false otherwise.
		 */
		virtual bool isEqual (std::vector<int> *set) = 0;

		/**
		 * Returns true if the given group is equal to this group.
		 *
		 * Uses interval notation, where rank i in the group represents
		 * beginRank + i in MPI_COMM_WORLD.
		 *
		 * @param beginRank of group.
		 * @param endRank of group.
		 * @return true if both groups are equal (processes and
		 *              their order matches), false otherwise.
		 */
		virtual bool isEqual (int beginRank, int endRank) = 0;

		/**
		 * Returns true if the given group is equal to this group.
		 *
		 * @param group to compare against.
		 * @return true if both groups are equal (processes and
		 *              their order matches), false otherwise.
		 */
		virtual bool isEqual (I_GroupTable* group) = 0;
	};
}/*namespace MUST*/

#endif /*I_GROUPTABLE_H*/
