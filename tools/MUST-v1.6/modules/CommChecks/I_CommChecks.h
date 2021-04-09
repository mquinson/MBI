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
 * @file I_CommChecks.h
 *       @see I_CommChecks.
 *
 *  @date 14.04.2011
 *  @author Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_CommTrack.h"

#ifndef I_COMMCHECKS_H
#define I_COMMCHECKS_H

/**
 * Interface for correctness checks of communicators.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 * - CommTrack
 */
class I_CommChecks : public gti::I_Module
{
public:

    /**
     *	Checks if an integer value (rank) is greater then the size of the communicator,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfGreaterCommSize (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int value,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if an integer value (rank) is greater then or equal to the size of the communicator,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
     *	@param lId location Id of the call site.
     *	@param aId argument Id of the integer to check.
     *	@param value to check.
     *	@param comm communicator of the call.
     *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfGreaterEqualCommSize (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int value,
    		MustCommType comm
    ) = 0;

    /**
     *	Checks if the product of all entries in the array is greater then the size of the communicator,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param array to check.
	 *	@param size size of the array that is checked
	 *	@param comm communicator  of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfProductGreaterCommSize (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const int* array,
    		int size,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if the product of all entries in the array is less then the size of the communicator,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param array to check.
	 *	@param size size of the array that is checked.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfProductLessCommSize (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const int* array,
    		int size,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if the communicator is known, if not,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotKnown (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if the communicator is MPI_COMM_NULL, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNull (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;
    		
    /**
     *	Checks if the communicator is an intercommunicator, if so,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfIsIntercomm (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if the communicator is an intercommunicator, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfIsIntercomm (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if the communicator has a cartesian topology, if not,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotCart (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if the communicator has a graph topology, if not,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotGraph (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;
    		
    /**
     *	Checks if the communicator has a cartesian or graph topology, if so,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfHasTopology (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;
    		
    /**
     *	Checks if the communicator is an intercommunicator and
     *	the MPI major version is 1, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfIsIntercommMPI1 (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if the communicator is an intercommunicator and
     *	the MPI major version is not 1, if so,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfIsIntercommMPI2 (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if an integer value (root) is greater then the size of the communicator,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId_root argument Id of the integer to check.
	 *	@param aId_comm argument Id of the communicator.
	 *	@param root value to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfRootNotInComm (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId_root,
    		int aId_comm,
    		int root,
    		MustCommType comm
    		) = 0;
    		
    /**
     *	Checks if the communicator is predefined, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfIsPredefined (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;
    		
    /**
     *	Checks if the communicator is an intercommunicator, if not,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotIntercomm (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if the communicator is MPI_COMM_NULL, if so,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the communicator to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfNull (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if an integer value (maxdims) is greater then the
     *	number of dimensions (ndims) of the given cartesian communicator,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId_root argument Id of the integer to check.
	 *	@param aId_comm argument Id of the communicator.
	 *	@param maxDims value to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningMaxDimsGreaterNDims (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId_maxdims,
    		int aId_comm,
    		int maxDims,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if an integer value (direction) is greater then the
     *	is actually an index into the dimensions of a cartesian communicator,
     *	i.e. direction < ndims,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId_direction argument Id of the integer to check.
	 *	@param aId_comm argument Id of the communicator.
	 *	@param direction value to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorDirectionGreaterNdims (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId_maxdims,
    		int aId_comm,
    		int direction,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if an integer value (maxneighbors) is smaller then the
     *	real number of neighbors in the graph communicator for this rank,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId_maxneighbors argument Id of the integer to check.
	 *	@param aId_rank argument Id of the rank.
	 *	@param aId_comm argument Id of the communicator.
	 *	@param maxneighbors value to check.
	 *	@param rank rank where the neighbors are quested.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningMaxNeighborsToSmall (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId_maxneighbors,
    		int aId_rank,
    		int aId_comm,
    		int maxneighbors,
    		int rank,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if an integer value (maxindices) is smaller then the
     *	count of indices in the graph communicator, if so,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId_maxindices argument Id of the integer to check.
	 *	@param aId_comm argument Id of the communicator.
	 *	@param maxindices value to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningMaxIndicesToSmall (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId_maxindices,
    		int aId_comm,
    		int maxindices,
    		MustCommType comm
    		) = 0;

    /**
     *	Checks if an integer value (maxedges) is smaller then the
     *	count of edges in the graph communicator, if so,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId_maxedges argument Id of the integer to check.
	 *	@param aId_comm argument Id of the communicator.
	 *	@param maxedges value to check.
	 *	@param comm communicator of the call.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningMaxEdgesToSmall (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId_maxedges,
    		int aId_comm,
    		int maxedges,
    		MustCommType comm
    		) = 0;

};/*class I_CommChecks*/

#endif /*I_COMMCHECKS_H*/
