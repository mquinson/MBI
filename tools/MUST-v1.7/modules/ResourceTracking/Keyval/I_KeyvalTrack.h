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
 * @file I_KeyvalTrack.h
 *       @see I_KeyvalTrack.
 *
 *  @date 12.05.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"

#include "MustEnums.h"
#include "BaseIds.h"
#include "MustTypes.h"
#include "I_Keyval.h"
#include "I_TrackBase.h"

#include <list>

#ifndef I_KEYVALTRACK_H
#define I_KEYVALTRACK_H

/**
 * Interface for querying information on keyval.
 *
 * Important: This analysis module only tracks keyvals,
 * it provides no correctness checking. However, it tries
 * to handle incorrect actions as good as possible.
 *
 * Dependencies (in listed order):
 * - ParallelIdAnalysis
 * - LocationAnalysis
 */
class I_KeyvalTrack : public gti::I_Module, public virtual must::I_TrackBase<must::I_Keyval>
{
public:

	/**
	 * Creates a new user defined keyval.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param commute true if commutative.
	 * @param keyval the newly created keyval.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN keyvalCreate (
			MustParallelId pId,
			MustLocationId lId,
			MustKeyvalType newKeyval) = 0;

	/**
	 * Frees the given keyval.
	 *
	 * @param pId parallel id of context.
	 * @param lId location id of context.
	 * @param keyval to free.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN keyvalFree (
			MustParallelId pId,
			MustLocationId lId,
			MustKeyvalType keyval) = 0;

	/**
	 * Returns a pointer to keyvalue information.
     * Is NULL if this is an unknown handle, note that
     * a MPI_INVALID_KEY handle returns a valid pointer though.
     *
     * Memory must not be freed and is valid until I_KeyvalTrack
     * receives the next event, if you need the information longer
     * query getPersistentKeyval instead.
	 *
	 * @param pId of the context.
	 * @param keyval to query for.
	 * @return information for the given keyval.
	 */
	virtual must::I_Keyval* getKeyval (
			MustParallelId pId,
			MustKeyvalType keyval) = 0;

	/** As I_KeyvalTrack::getKeyval with rank instead of pid.*/
	virtual must::I_Keyval* getKeyval (
			int rank,
			MustKeyvalType comm) = 0;

	/**
	 * Like I_KeyvalTrack::getKeyval, though returns a persistent information
     * that is valid until you erase it, i.e.:
     *@code
     I_KeyvalPersistent* keyvalInfo = myKeyvalTrack->getPersistentKeyval (pId, handle);
     if (keyvalInfo == NULL) return;
     .... //Do something with keyvalInfo
     keyvalInfo->erase(); //Mark as not needed any longer
     *@endcode
     *
     * A reference count mechanism is used to implement this.
	 *
	 * @param pId of the context.
	 * @param keyval to query for.
	 * @return information for the given keyval.
	 */
	virtual must::I_KeyvalPersistent* getPersistentKeyval (
	        MustParallelId pId,
	        MustKeyvalType keyval) = 0;

	/** As I_KeyvalTrack::getPersistentKeyval with rank instead of pid.*/
	virtual must::I_KeyvalPersistent* getPersistentKeyval (
	        int rank,
	        MustKeyvalType comm) = 0;

	/**
	 * Adds the integer (MustKeyvalType) values for all predefined
	 * (named) handles for keyvals.
	 *
     * @param pId of the context.
	 * @param keyvalNull value of MPI_KEYVAL_INVALID.
	 * @param numPredefs number of predefined non null keyvals being sent.
	 * @param predefinedIds array of value of MustMpiKeyvalPredefined for each predefined type, array size is numPredefs.
	 * @param predefinedValues array of handles for the predefined types.
	 *
	 */
	virtual gti::GTI_ANALYSIS_RETURN addPredefineds (
            MustParallelId pId,
			MustKeyvalType keyvalNull,
			int numPredefs,
			int* predefinedIds,
			MustKeyvalType* predefinedValues) = 0;

	/**
	 * Returns a list of all currently known user handles.
	 * Usage scenarios involve logging lost handles at finalize.
	 * @return a list of pairs of the form (rank, handle id).
	 */
	virtual std::list<std::pair<int, MustKeyvalType> > getUserHandles (void) = 0;

	/**
	 * Allows other modules to notify this module of an ongoing shutdown.
	 * This influcences the behavior of passing free calls across to other layers.
	 */
	virtual void notifyOfShutdown (void) = 0;
        virtual bool isPredefined(must::I_Keyval * info) {return false;}    
        
}; /*class I_KeyvalTrack*/

#endif /*I_KEYVALTRACK_H*/
