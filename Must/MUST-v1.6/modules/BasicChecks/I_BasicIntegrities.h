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
 * @file I_BasicIntegrities.h
 *       @see I_BasicIntegrities.
 *
 *  @date 10.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_CommTrack.h"

#ifndef I_BASICINTEGRITIES_H
#define I_BASICINTEGRITIES_H

/**
 * Interface for basic integrity checks.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 *
 */
class I_BasicIntegrities : public gti::I_Module
{
public:

	/**
	 * If the given number is > 0, checks whether the given pointer is not NULL,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param size of the allocated region, if > 0, the given pointer must not be NULL.
	 * @param pointer to not be null if the given size is > 0.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullCondition (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int size,
    		const void* pointer) = 0;

	/**
	 * Checks whether the given pointer is not NULL,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param pointer to not be null.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNull (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const void* pointer) = 0;

	/**
	 * If the size of the communicator is > 0, checks whether the given pointer is not NULL,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param comm communicator of this call.
	 * @param pointer to not be null if the given size is > 0.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullCommSize (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm,
    		const void* pointer) = 0;


	/**
	 * If the given array contains a entry that is > 0
	 * and the size of the communicator is > 0,
	 * checks whether the given pointer is not NULL or MPI_BOTTOM,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param size of the allocated region, if > 0, the given pointer must not be NULL.
	 * @param pointer to not be null if the given size is > 0.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottomConditionCommSize (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const int* array,
    		MustCommType comm,
    		const void* pointer) = 0;


	/**
	 * If the given number is > 0, checks whether
	 * the given pointer is not NULL or MPI_BOTTOM,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param size of the allocated region, if > 0, the given pointer must not be NULL.
	 * @param pointer to not be null if the given size is > 0.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottom (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int size,
    		const void* pointer) = 0;


	/**
	 * Checks whether the given pointer is not NULL,
	 * if violated it manifests as warning.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param pointer to not be null.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN warningIfNull (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const void* pointer) = 0;


	/**
	 * Checks whether for value of OMP_NUM_THREADS>1 while
	 * low provided threadlevel.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param requested threadlevel requested by application.
	 * @param provided threadlevel provided by application.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN warningForLowThreadlevel (
    		MustParallelId pId,
    		MustLocationId lId,
    		int requested,
    		int provided) = 0;


	/**
	 * If the given array is > 0 at the index of this processes rank in the given comm
	 * and the size of the communicator is > 0,
	 * checks whether the given pointer is not NULL or MPI_BOTTOM,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param size of the allocated region, if > 0, the given pointer must not be NULL.
	 * @param pointer to not be null if the given size is > 0.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottomAtIndexCommSize (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const int* array,
    		MustCommType comm,
    		const void* pointer) = 0;

	/**
	 * Checks whether the given pointer is NULL (while not being MPI_STATUS_IGNORE, if defined),
	 * if so manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param pointer to check.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullStatus (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const void* pointer) = 0;

	/**
	 * Checks whether the given pointer is NULL (while not being MPI_STATUSES_IGNORE, if defined),
	 * if so manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param size of pointer array
	 * @param pointer to check.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullStatuses (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const void* pointer) = 0;


	/**
	 * If the given number is > 0 at root node, checks whether
	 * the given pointer is not NULL or MPI_BOTTOM,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param size of the allocated region, if > 0, the given pointer must not be NULL.
	 * @param pointer to not be null if the given size is > 0.
     * @param root collecting node
     * @param comm communicator for communication
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottomOnlyOnRoot (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int size,
    		const void* pointer,
    		int root,
    		MustCommType comm
			) = 0;

	/**
	 * Checks whether the given pointer is not NULL at root node,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param pointer to not be null.
     * @param root collecting node
     * @param comm communicator for communication
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullOnlyOnRoot (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const void* pointer,
    		int root,
    		MustCommType comm
			) = 0;

	/**
	 * If the given array contains a entry that is > 0
	 * and the size of the communicator is > 0 at root node,
	 * checks whether the given pointer is not NULL or MPI_BOTTOM,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param size of the allocated region, if > 0, the given pointer must not be NULL.
	 * @param pointer to not be null if the given size is > 0.
     * @param comm communicator for communication
     * @param root collecting node
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullAndNotMpiBottomConditionCommSizeOnlyOnRoot (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const int* array,
    		MustCommType comm,
    		const void* pointer,
    		int root
    		) = 0;

	/**
	 * If the size of the communicator is > 0 at root node, checks whether the given pointer is not NULL,
	 * if violated it manifests as error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param comm communicator of this call.
	 * @param pointer to not be null if the given size is > 0.
         * @param root collecting node
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNullCommSizeOnlyOnRoot (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm,
    		const void* pointer,
    		int root) = 0;

	/**
	 * For some collectives MPI_IN_PLACE only makes sense on non-root ranks
	 * This function raises error is used there.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param comm communicator of this call.
	 * @param address pointer
     * @param root collecting node
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfInPlaceOtherThanRoot (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm,
    		MustAddressType pointer,
    		int root) = 0;


	/**
	 * If a buffer is set to MPI_IN_PLACE, this function raises error.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param aId argument Id of the value to check.
	 * @param address pointer
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfInPlaceUsed (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustAddressType pointer) = 0;
                                                
};/*class I_BasicIntegrities*/

#endif /*I_BASICINTEGRITIES_H*/
