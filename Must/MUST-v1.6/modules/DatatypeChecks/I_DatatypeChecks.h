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
 * @file I_DatatypeChecks.h
 *       @see I_DatatypeChecks.
 *
 *  @date 23.05.2011
 *  @author Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_DatatypeTrack.h"

#ifndef I_DATATYPECHECKS_H
#define I_DATATYPECHECKS_H

/**
 * Interface for correctness checks of datatypes.
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - CreateMessage
 * - ArgumentAnalysis
 * - DatatypeTrack
 *
 */
class I_DatatypeChecks : public gti::I_Module
{
public:

    /**
     * Checks if a datatype is unknown,
     * manifests as error
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotKnown (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            MustDatatypeType datatype) = 0;

    /**
     * Checks if a datatype is null,
     * manifests as error
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNull (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            MustDatatypeType datatype) = 0;

    /**
     * Checks if a datatype is not commited,
     * !(isPredefined || isCommited)
     * manifests as error
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotCommited (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            MustDatatypeType datatype) = 0;

    /**
     * Checks if a datatype is commited,
     * (isPredefined || isCommited)
     * manifests as warning
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfCommited (
            MustParallelId pId,
            MustLocationId lId,
            int aId,
            MustDatatypeType datatype) = 0;

    /**
     * Checks if a datatype is propperly aligned,
     * manifests as warning
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN warningIfNotPropperlyAligned (
            MustParallelId pId,
            MustLocationId lId,
            int aId,
            MustDatatypeType datatype) = 0;

    /**
     * Checks if a datatype is valid for use in communication calls,
     * (errorIfNotKnown || errorIfNull || errorIfNotCommited)
     * manifests as error
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotValidForCommunication (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            MustDatatypeType datatype) = 0;

    /**
     * Checks if a array of datatype is valid for use in communication calls,
     * (errorIfNotKnown || errorIfNull || errorIfNotCommited)
     * manifests as error
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfArrayNotValidForCommunication (
            MustParallelId pId,
            MustLocationId lId,
            int aId,
            MustDatatypeType* datatypes,
            int count) = 0;

    /**
     * Checks if a datatype is valid for creation of a new datatype
     * (errorIfNotKnown || errorIfNull)
     * manifests as error
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotValidForCreate (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            MustDatatypeType datatype) = 0;

    /**
     * Checks if a array of datatypes is valid for creation of a new datatype
     * for_each(errorIfNotKnown || errorIfNull)
     * manifests as error
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatypes[] to check.
     * @param count size of the array
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfArrayNotValidForCreate (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            MustDatatypeType* datatypes,
            int count) = 0;

    /**
     * Checks if a datatype is valid for creation of a new datatype
     * (errorIfNotKnown || errorIfNull || warningIfCommited)
     * manifests as error or warning
     *
     * @param pId parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param aId argument Id of the value to check.
     * @param datatype to check.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN errorIfNotValidForCommit (
            MustParallelId pId, 
            MustLocationId lId, 
            int aId, 
            MustDatatypeType datatype) = 0;

};/*class I_DatatypeChecks*/

#endif /*I_DATATYPECHECKS_H*/
