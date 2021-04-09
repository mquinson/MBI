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
 * @file I_ErrTrack.h
 *       @see I_ErrTrack.
 *
 *  @date 12.05.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"
#include "I_Err.h"
#include "I_TrackBase.h"

#include <list>

#ifndef I_ERRTRACK_H
#define I_ERRTRACK_H

/**
 * Interface for querying information on ers.
 *
 * Important: This analysis module only tracks errs,
 * it provides no correctness checking. However, it tries
 * to handle incorrect actions as good as possible.
 *
 * Dependencies (in listed order):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 */
class I_ErrTrack : public gti::I_Module, public virtual must::I_TrackBase<must::I_Err>
{
public:

	/**
	 * Creates a new user defined err.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param commute true if commutative.
	 * @param err the newly created err.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN errCreate (
			MustParallelId pId,
			MustLocationId lId,
			MustErrType newErr) = 0;

	/**
	 * Frees the given err.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param err to free.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN errFree (
			MustParallelId pId,
			MustLocationId lId,
			MustErrType err) = 0;

	/**
	 * Returns a pointer to the errorhandler information.
     * Is NULL if this is an unknown handle, note that
     * an MPI_ERRHANDLER_NULL handle returns a valid pointer though.
     *
     * Memory must not be freed and is valid until I_ErrTrack
     * receives the next event, if you need the information longer
     * query getPersistentErr instead.
	 *
	 * @param pId of the context.
	 * @param err to query for.
	 * @return information for the given err.
	 */
	virtual must::I_Err* getErr (
			MustParallelId pId,
			MustErrType err) = 0;

	/** As I_ErrTrack::getErr with rank instead of pid.*/
	virtual must::I_Err* getErr (
			int rank,
			MustErrType err) = 0;

	/**
	 * Like I_ErrTrack::getErr, though returns a persistent information
     * that is valid until you erase it, i.e.:
     *@code
     I_ErrPersistent * errInfo = myErrTrack->getPersistentErr (pId, handle);
     if (errInfo == NULL) return;
     .... //Do something with errInfo
     errInfo->erase(); //Mark as not needed any longer
     *@endcode
     *
     * A reference count mechanism is used to implement this.
	 *
	 * @param pId of the context.
	 * @param err to query for.
	 * @return information for the given err.
	 */
	virtual must::I_ErrPersistent* getPersistentErr (
	        MustParallelId pId,
	        MustErrType err) = 0;

	/** As I_ErrTrack::getPersistentErr with rank instead of pid.*/
	virtual must::I_ErrPersistent* getPersistentErr (
	        int rank,
	        MustErrType err) = 0;

	/**
	 * Adds the integer (MustErrType) values for all predefined
	 * (named) handles for errs.
	 *
     * @param pId of the context.
	 * @param errNull value of MPI_ERRHANDLER_NULL.
	 * @param numPredefs number of predefined non null errs being sent.
	 * @param predefinedIds array of value of MustMpiErrPredefined for each predefined type, array size is numPredefs.
	 * @param predefinedValues array of handles for the predefined types.
	 *
	 */
	virtual gti::GTI_ANALYSIS_RETURN addPredefineds (
            MustParallelId pId,
			MustErrType errNull,
			int numPredefs,
			int* predefinedIds,
			MustErrType* predefinedValues) = 0;

	/**
	 * Returns a list of all currently known user handles.
	 * Usage scenarios involve logging lost handles at finalize.
	 * @return a list of pairs of the form (rank, handle id).
	 */
	virtual std::list<std::pair<int, MustErrType> > getUserHandles (void) = 0;

	/**
	 * Allows other modules to notify this module of an ongoing shutdown.
	 * This influcences the behavior of passing free calls across to other layers.
	 */
	virtual void notifyOfShutdown (void) = 0;
        virtual bool isPredefined(must::I_Err * info) {return false;}    
        
}; /*class I_ErrTrack*/

#endif /*I_ERRTRACK_H*/
