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
 * @file I_IntegerChecks.h
 *       @see I_IntegerChecks.
 *
 *  @date 01.03.2011
 *  @author Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"

#ifndef I_INTEGERCHECKS_H
#define I_INTEGERCHECKS_H

/**
 * Interface for correctness checks for integers.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 * - BaseConstants
 *
 */
class I_IntegerChecks : public gti::I_Module
{
public:
	/**
	 *	Checks if a integer value is less than 0,
	 *	manifests as error.
	 *
	 *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfLessThanZero (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int value) = 0;

    /**
     *	Checks if a integer value is 0,
     *	manifests as warning.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfZero (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int value) = 0;

    /**
     *	Checks if a integer value is 0,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfZero (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int value) = 0;

    /**
     *	Checks if a integer value is not 0 or 1,
     *	manifests as warning.
     *	Used for arguments that are marked as
     *	logical in the MPI standard.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfNotOneOrZero (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		int value) = 0;

	/**
	 *	Checks if an integer array has an entry that is less than 0,
	 *	manifests as error.
	 *
	 *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param array to check.
	 *	@param size of array
	 *	@return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN errorIfLessThanZeroArray (
    		MustParallelId pId, 
    		MustLocationId lId,
    		int aId,
    		const int* array,
    		int size) = 0;

    /**
    	 *	Checks if an integer array has a entry that is 0,
    	 *	manifests as warning.
    	 *
    	 *	@param pId parallel Id of the call site.
    	 *	@param lId location Id of the call site.
    	 *	@param aId argument Id of the integer to check.
    	 *	@param array to check.
    	 *	@param size of array
    	 *	@return see gti::GTI_ANALYSIS_RETURN.
    	 */
	virtual gti::GTI_ANALYSIS_RETURN warningIfZeroArray (
		MustParallelId pId, 
		MustLocationId lId,
		int aId,
		const int* array,
		int size) = 0;

    /**
     *	Checks if an integer array contains a value that is not 0 or 1,
     *	manifests as warning.
     *	Used for arguments that are marked as
     *	logical in the MPI standard.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param array to check.
	 *	@param size of array
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfNotOneOrZeroArray (
    		MustParallelId pId, 
    		MustLocationId lId, 
    		int aId, 
    		const int* array,
    		int size) = 0;



    /**
    	 *	Checks if an all entries in an integer Array are smaller than a specified integer, if not, 
    	 *	manifests as error.
    	 *
    	 *	@param pId parallel Id of the call site.
    	 *	@param lId location Id of the call site.
    	 *	@param aId argument Id of the integer to check.
    	 *	@param array to check.
    	 *	@param size of array
    	 *	@param border all array entries must be smaller than this number
    	 *	@return see gti::GTI_ANALYSIS_RETURN.
    	 */
	virtual gti::GTI_ANALYSIS_RETURN errorIfEntryIsGreaterOrEqualArray (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		const int* array,
		int size,
		int border) = 0;

    /**
     *	Checks if an integer value is negative while not being MPI_PROC_NULL 
     *	or MPI_ANY_SOURCE, if so, 
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNegativNotProcNullAnySource (
    		MustParallelId pId, 
    		MustLocationId 
    		lId, 
    		int aId, 
    		int value) = 0;

    /**
     *	Checks if an integer value is negative while not being MPI_PROC_NULL, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNegativNotProcNull (
    		MustParallelId pId, 
    		MustLocationId lId, 
    		int aId, 
    		int value) = 0;

    /**
     *	Checks if an integer value is within a range of 0 to MPI_TAG_UB, if not,
     *	manifests as error.
     *	Used for tags of send calls.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check (tag).
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotWithinRangeZeroAndLessTagUb (
    		MustParallelId pId, 
    		MustLocationId lId, 
    		int aId, 
    		int value) = 0;

    /**
     *	Checks if an integer value is within a range of 0 to MPI_TAG_UB or equals MPI_ANY_TAG, if not,
     *	manifests as error.
     *	Used for tags of receive calls.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check (tag).
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotWithinRangeZeroAndLessTagUbAnyTag (
    		MustParallelId pId, 
    		MustLocationId lId, 
    		int aId, 
    		int value) = 0;

    /**
     *	Checks if an integer value is within a range of 32767 to MPI_TAG_UB, if so,
     *	manifests as warning.
     *	Used for checking tags.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfIsHighButLessTagUb (
    		MustParallelId pId, 
    		MustLocationId lId, 
    		int aId, 
    		int value) = 0;

    /**
     *	Checks if any value of an array negative while not being MPI_PROC_NULL, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param array to check.
	 *	@param size of array
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNegativNotProcNullArray (
    		MustParallelId pId, 
    		MustLocationId lId, 
    		int aId, 
    		const int* array,
    		int size) = 0;

    /**
     *	Checks if an integer value is negative, equals MPI_PROC_NULL,
     *	or MPI_ANY_SOURCE, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNegativProcNullAnySource (
    		MustParallelId pId, 
    		MustLocationId lId, 
    		int aId, 
    		int value) = 0;

    /**
     *	Checks if an integer value is negative and while not being MPI_UNDEFINED, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer to check.
	 *	@param value to check.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNegativNotUndefined (
    		MustParallelId pId, 
    		MustLocationId lId, 
    		int aId, 
    		int value) = 0;


    /**
     *	Checks if an integer array contains duplicated entries, if so,
     *	manifests as error.
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer array to check.
	 *	@param array to check.
	 *	@param size of array.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfDuplicateRank (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const int* array,
    		int size) = 0;


    /**
     *	Checks if an array of integer triplets of the form (first rank, last rank, stride) is correct, if not
   	 *	manifests as error.
   	 *	This includes:
   	 *	- contains duplicated ranks
   	 *	- Strides < 0 || >0 (!=0)
     *
     *	@param pId parallel Id of the call site.
	 *	@param lId location Id of the call site.
	 *	@param aId argument Id of the integer array to check.
	 *	@param array to check.
	 *	@param size of array.
	 *	@return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN checkGroupRangeArray (
    		MustParallelId pId,
    		MustLocationId lId,
    		int aId,
    		const int* array,
    		int size) = 0;

};

#endif /*I_INTEGERCHECKS_H*/
