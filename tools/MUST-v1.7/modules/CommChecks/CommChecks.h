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
 * @file CommChecks.h
 *       @see MUST::CommChecks.
 *
 *  @date 14.04.2011
 *  @author Mathias Korepkat
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"
#include "I_CommChecks.h"
#include "I_BaseConstants.h"

#include <string>

#ifndef COMMCHECKS_H
#define COMMCHECKS_H

using namespace gti;

namespace must
{
	/**
     * CommChecks for correctness checks of communicators, interface implementation.
     */
    class CommChecks : public gti::ModuleBase<CommChecks, I_CommChecks>
    {
    public:
			/**
			 * Constructor.
			 * @param instanceName name of this module instance.
			 */
			CommChecks (const char* instanceName);

			/**
			 * Destructor.
			 */
			virtual ~CommChecks (void);

			/**
			 * @see I_CommChecks::errorIfGreaterCommSize.
			 */
			GTI_ANALYSIS_RETURN errorIfGreaterCommSize (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					int value,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfGreaterEqualCommSize.
			 */
			GTI_ANALYSIS_RETURN errorIfGreaterEqualCommSize (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					int value,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfProductGreaterCommSize.
			 */
			GTI_ANALYSIS_RETURN errorIfProductGreaterCommSize (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					const int* array,
					int size,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::warningIfProductLessCommSize.
			 */
			GTI_ANALYSIS_RETURN warningIfProductLessCommSize (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					const int* array,
					int size,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfNotKnown.
			 */
			GTI_ANALYSIS_RETURN errorIfNotKnown (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfNull.
			 */
			GTI_ANALYSIS_RETURN errorIfNull (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);
			/**
			 * @see I_CommChecks::warningIfIsIntercomm.
			 */
			GTI_ANALYSIS_RETURN warningIfIsIntercomm (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfIsIntercomm.
			 */
			GTI_ANALYSIS_RETURN errorIfIsIntercomm (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfNotCart.
			 */
			GTI_ANALYSIS_RETURN errorIfNotCart (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfNotGraph.
			 */
			GTI_ANALYSIS_RETURN errorIfNotGraph(
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::warningIfHasTopology.
			 */
			GTI_ANALYSIS_RETURN warningIfHasTopology(
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);
			/**
			 * @see I_CommChecks::errorIfIsIntercommMPI1.
			 */
			GTI_ANALYSIS_RETURN errorIfIsIntercommMPI1 (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::warningIfIsIntercommMPI2.
			 */
			GTI_ANALYSIS_RETURN warningIfIsIntercommMPI2 (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfRootNotInComm.
			 */
			GTI_ANALYSIS_RETURN errorIfRootNotInComm (
					MustParallelId pId,
					MustLocationId lId,
					int aId_root,
					int aId_comm,
					int root,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfIsPredefined.
			 */
			GTI_ANALYSIS_RETURN errorIfIsPredefined (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorIfNotIntercomm.
			 */
			GTI_ANALYSIS_RETURN errorIfNotIntercomm (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::warningIfNull.
			 */
			GTI_ANALYSIS_RETURN warningIfNull (
					MustParallelId pId,
					MustLocationId lId,
					int aId,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::warningMaxDimsGreaterNDims.
			 */
			GTI_ANALYSIS_RETURN warningMaxDimsGreaterNDims  (
					MustParallelId pId,
					MustLocationId lId,
					int aId_maxdims,
					int aId_comm,
					int maxDims,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::errorDirectionGreaterNdims.
			 */
			GTI_ANALYSIS_RETURN errorDirectionGreaterNdims (
					MustParallelId pId,
					MustLocationId lId,
					int aId_direction,
					int aId_comm,
					int direction,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::warningMaxNeighborsToSmall.
			 */
			GTI_ANALYSIS_RETURN warningMaxNeighborsToSmall (
					MustParallelId pId,
					MustLocationId lId,
					int aId_maxneighbors,
					int aId_rank,
					int aId_comm,
					int maxneighbors,
					int rank,
					MustCommType comm
			);


			/**
			 * @see I_CommChecks::warningMaxIndicesToSmall.
			 */
			GTI_ANALYSIS_RETURN warningMaxIndicesToSmall (
					MustParallelId pId,
					MustLocationId lId,
					int aId_maxindices,
					int aId_comm,
					int maxindices,
					MustCommType comm
			);

			/**
			 * @see I_CommChecks::warningMaxEdgesToSmall.
			 */
			GTI_ANALYSIS_RETURN warningMaxEdgesToSmall (
					MustParallelId pId,
					MustLocationId lId,
					int aId_maxedges,
					int aId_comm,
					int maxedges,
					MustCommType comm
			);

    protected:
			I_ParallelIdAnalysis* myPIdMod;
			I_CreateMessage* myLogger;
			I_ArgumentAnalysis* myArgMod;
			I_CommTrack* myCommMod;
			I_BaseConstants* myConstMod;
    };
} /*namespace MUST*/

#endif /*COMMCHECKS_H*/
