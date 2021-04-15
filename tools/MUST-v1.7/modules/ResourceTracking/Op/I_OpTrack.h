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
 * @file I_OpTrack.h
 *       @see I_OpTrack.
 *
 *  @date 10.05.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"
#include "I_Op.h"
#include "I_TrackBase.h"

#include <list>

#ifndef I_OPTRACK_H
#define I_OPTRACK_H

/**
 * Interface for querying information on operations.
 *
 * Important: This analysis module only tracks ops,
 * it provides no correctness checking. However, it tries
 * to handle incorrect actions as good as possible.
 *
 * Dependencies (in listed order):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 */
class I_OpTrack : public gti::I_Module, public virtual must::I_TrackBase<must::I_Op>
{
public:

	/**
	 * Creates a new user defined op.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param commute true if commutative.
	 * @param op the newly created operation.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN opCreate (
			MustParallelId pId,
			MustLocationId lId,
			int commute,
			MustOpType newOp) = 0;

	/**
	 * Frees the given op.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param op to free.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN opFree (
			MustParallelId pId,
			MustLocationId lId,
			MustOpType op) = 0;

	/**
	 * Returns pointer to operation information.
     * Is NULL if this is an unknown handle, note that
     * an MPI_OP_NULL handle returns a valid pointer though.
     *
     * Memory must not be freed and is valid until I_OpTrack
     * receives the next event, if you need the information longer
     * query getPersistentOp instead.
	 *
	 * @param pId of the context.
	 * @param op to query for.
	 * @return information for the given op.
	 */
	virtual must::I_Op* getOp (
			MustParallelId pId,
			MustOpType op) = 0;

	/** As I_OpTrack::getOp with rank instead of pid.*/
	virtual must::I_Op* getOp (
			int rank,
			MustOpType op) = 0;

    /**
     * Like I_OpTrack::getOp though returns a persistent information
     * that is valid until you erase it, i.e.:
     *@code
     I_OpPersistent * opInfo = myOpTrack->getPersistentOp (pId, handle);
     if (opInfo == NULL) return;
     .... //Do something with opInfo
     opInfo->erase(); //Mark as not needed any longer
     *@endcode
     *
     * A reference count mechanism is used to implement this.
     *
     * @param pId of the context.
     * @param op to query for.
     * @return information for the given op.
     */
    virtual must::I_OpPersistent* getPersistentOp (
            MustParallelId pId,
            MustOpType op) = 0;

    /** As I_OpTrack::getPersistentOp with rank instead of pid.*/
    virtual must::I_OpPersistent* getPersistentOp (
            int rank,
            MustOpType op) = 0;

	/**
	 * Adds the integer (MustOpType) values for all predefined
	 * (named) handles for ops.
	 *
     * @param pId of the context.
	 * @param opNull value of MPI_OP_NULL.
	 * @param numPredefs number of predefined non null ops being sent.
	 * @param predefinedIds array of value of MustMpiOpPredefined for each predefined type, array size is numPredefs.
	 * @param predefinedValues array of handles for the predefined types.
	 *
	 */
	virtual gti::GTI_ANALYSIS_RETURN addPredefineds (
            MustParallelId pId,
			MustOpType opNull,
			int numPredefs,
			int* predefinedIds,
			MustOpType* predefinedValues) = 0;

	/**
	 * Returns a list of all currently known user handles.
	 * Usage scenarios involve logging lost handles at finalize.
	 * @return a list of pairs of the form (rank, handle id).
	 */
	virtual std::list<std::pair<int, MustOpType> > getUserHandles (void) = 0;

	/**
	 * Allows other modules to notify this module of an ongoing shutdown.
	 * This influcences the behavior of passing free calls across to other layers.
	 */
	virtual void notifyOfShutdown (void) = 0;
        virtual bool isPredefined(must::I_Op * type) {return false;}    
        
}; /*class I_OpTrack*/

#endif /*I_OPTRACK_H*/
