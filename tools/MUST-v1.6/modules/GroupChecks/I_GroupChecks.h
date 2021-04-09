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
 * @file I_GroupChecks.h
 *       @see I_GroupChecks.
 *
 *  @date 23.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_GROUPCHECKS_H
#define I_GROUPCHECKS_H

/**
 * Interface for correctness checks of groups.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 * - CommTrack
 * - GroupTrack
 *
 */
class I_GroupChecks : public gti::I_Module
{
public:

	/**
	 * Checks if the group is known, if not,
     * manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param group argument to check
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotKnown (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustGroupType group
    		) = 0;

    /**
    * Checks if the group is NULL, if so,
    * manifests as error.
    *
    * @param pId parallel Id of the call site.
    * @param lId location Id of the call site.
    * @param aId argument Id of the value to check.
    * @param group argument to check
    * @return see gti::GTI_ANALYSIS_RETURN.
    */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNull (
      		MustParallelId pId,
       		MustLocationId lId,
       		int aId,
       		MustGroupType group
       		) = 0;

    /**
    * Checks if an integer is greater than the size of a given group,
    * if so, manifests as error.
    *
    * @param pId parallel Id of the call site.
    * @param lId location Id of the call site.
    * @param aId_val argument Id of the value to check.
    * @param aId_grp argument Id of the group to check.
    * @param value argument to check.
    * @param group argument to get size of.
    * @return see gti::GTI_ANALYSIS_RETURN.
    */
    virtual gti::GTI_ANALYSIS_RETURN errorIfIntegerGreaterGroupSize (
      		MustParallelId pId,
       		MustLocationId lId,
       		int aId_val,
       		int aId_grp,
       		int value,
       		MustGroupType group
       		) = 0;

    /**
    * Checks if one or more elements of an integer array are
    * greater then groupsize,
    * if so, manifests as error.
    *
    * @param pId parallel Id of the call site.
    * @param lId location Id of the call site.
    * @param aId_val argument Id of the value to check.
    * @param aId_grp argument Id of the group to check.
    * @param array argument to check.
    * @param size of array.
    * @param group argument to get size of.
    * @return see gti::GTI_ANALYSIS_RETURN.
    */
    virtual gti::GTI_ANALYSIS_RETURN errorIfIntegerArrayElementGreaterGroupSize (
      		MustParallelId pId,
       		MustLocationId lId,
       		int aId_val,
       		int aId_grp,
       		const int* array,
       		int size,
       		MustGroupType group
       		) = 0;

    /**
    * Checks if a rank from a range array triplet (first rank, last rank, stride)
    * is not in a given group,
    * if so, manifests as error.
    *
    * @param pId parallel Id of the call site.
    * @param lId location Id of the call site.
    * @param aId_val argument Id of the value to check.
    * @param aId_grp argument Id of the group to check.
    * @param array argument to check
    * @param size of array
    * @param group argument to get size of
    * @return see gti::GTI_ANALYSIS_RETURN.
    */
    virtual gti::GTI_ANALYSIS_RETURN errorIfRankFromRangesNotInGroup (
      		MustParallelId pId,
       		MustLocationId lId,
       		int aId_val,
       		int aId_grp,
       		const int* array,
       		int size,
       		MustGroupType group
       		) = 0;

    /**
    * Checks if group is empty, if so
    * manifests as warning.
    *
    * @param pId parallel Id of the call site.
    * @param lId location Id of the call site.
    * @param aId argument Id of the group to check.
    * @param group argument to get size of
    * @return see gti::GTI_ANALYSIS_RETURN.
    */
    virtual gti::GTI_ANALYSIS_RETURN warningIfEmpty (
      		MustParallelId pId,
       		MustLocationId lId,
       		int aId,
       		MustGroupType group
       		) = 0;

    /**
    * Checks if group is null, if so
    * manifests as warning.
    *
    * @param pId parallel Id of the call site.
    * @param lId location Id of the call site.
    * @param aId argument Id of the group to check.
    * @param group argument to get size of
    * @return see gti::GTI_ANALYSIS_RETURN.
    */
    virtual gti::GTI_ANALYSIS_RETURN warningIfNull (
      		MustParallelId pId,
       		MustLocationId lId,
       		int aId,
       		MustGroupType group
       		) = 0;

    /**
    * Checks if ranks of a group are in a certain communicator,
    * if so, manifests as error.
    *
    * @param pId parallel Id of the call site.
    * @param lId location Id of the call site.
    * @param aId_grp argument Id of the group to check.
    * @param aId_comm argument Id of the group to check.
    * @param group argument to check
    * @param comm argument to check
    * @return see gti::GTI_ANALYSIS_RETURN.
    */
    virtual gti::GTI_ANALYSIS_RETURN errorRankNotInComm (
      		MustParallelId pId,
       		MustLocationId lId,
       		int aId_grp,
       		int aId_comm,
       		MustGroupType group,
       		MustCommType comm
       		) = 0;


};/*class I_GroupChecks*/

#endif /*I_GROUPCHECKS_H*/
