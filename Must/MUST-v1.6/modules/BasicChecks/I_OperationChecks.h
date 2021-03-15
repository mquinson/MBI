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
 * @file I_OperationChecks.h
 *       @see I_OperationChecks.
 *
 *  @date 26.05.2011
 *  @author Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_OpTrack.h"

#ifndef I_OPERATIONCHECKS_H
#define I_OPERATIONCHECKS_H

/**
 * Interface for correctness checks for operations.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 * - OpTrack
 *
 */
class I_OperationChecks : public gti::I_Module
{
public:
	/**
	 *	Checks if a operatation is predefined, if so,
	 *	manifests as error.
	 *
	 *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param op operation to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfPredefined (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustOpType op
    		) = 0;

	/**
	 *	Checks if a operatation is unknown, if so,
	 *	manifests as error.
	 *
	 *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param op operation to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotKnown (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustOpType op
    		) = 0;

	/**
	 *	Checks if a operatation is MPI_OP_NULL, if so,
	 *	manifests as error.
	 *
	 *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param op operation to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNull (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		MustOpType op
    		) = 0;

};

#endif /*I_OPERATIONCHECKS_H*/
