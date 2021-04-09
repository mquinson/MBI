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
 * @file DatatypeChecks.h
 *       @see MUST::DatatypeChecks.
 *
 *  @date 23.05.2011
 *  @author Joachim Protze
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_ArgumentAnalysis.h"
#include "I_CreateMessage.h"

#include "I_DatatypeChecks.h"

#include <string>

#ifndef DATATYPECHECKS_H
#define DATATYPECHECKS_H

using namespace gti;

namespace must
{
    /**
     * DatatypeChecks for correctness checks interface implementation.
     */
    class DatatypeChecks : public gti::ModuleBase<DatatypeChecks, I_DatatypeChecks>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
            DatatypeChecks (const char* instanceName);

            /**
             * Destructor.
             */
            virtual ~DatatypeChecks (void);

            /**
             * @see I_DatatypeChecks::errorIfNotKnown.
             */
    public:
            GTI_ANALYSIS_RETURN errorIfNotKnown (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);
    private:
            // same as above, with resolved datatype and optional index for datatype-arrays
            GTI_ANALYSIS_RETURN errorIfNotKnown (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    I_Datatype * info,
                    int index=-1);

            /**
             * @see I_DatatypeChecks::errorIfNull.
             */
    public:
            GTI_ANALYSIS_RETURN errorIfNull (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);
    private:
            // same as above, with resolved datatype and optional index for datatype-arrays
            GTI_ANALYSIS_RETURN errorIfNull (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    I_Datatype * info,
                    int index=-1);

            /**
             * @see I_DatatypeChecks::errorIfNotCommited.
             */
    public:
            GTI_ANALYSIS_RETURN errorIfNotCommited (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);
    private:
            // same as above, with resolved datatype and optional index for datatype-arrays
            GTI_ANALYSIS_RETURN errorIfNotCommited (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    I_Datatype * info,
                    MustDatatypeType& datatype,
                    int index=-1);


            /**
             * @see I_DatatypeChecks::warningIfCommited.
             */
    public:
            GTI_ANALYSIS_RETURN warningIfCommited (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);
    private:
            // same as above, with resolved datatype and optional index for datatype-arrays
            GTI_ANALYSIS_RETURN warningIfCommited (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    I_Datatype * info,
                    MustDatatypeType& datatype,
                    int index=-1);

            /**
             * @see I_DatatypeChecks::warningIfNotPropperlyAligned.
             */
    public:
            GTI_ANALYSIS_RETURN warningIfNotPropperlyAligned (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);
    private:
            // same as above, with resolved datatype and optional index for datatype-arrays
            GTI_ANALYSIS_RETURN warningIfNotPropperlyAligned (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    I_Datatype * info,
                    MustDatatypeType& datatype,
                    int index=-1);

    public:
            /**
             * @see I_DatatypeChecks::errorIfNotValidForCommunication.
             */
            GTI_ANALYSIS_RETURN errorIfNotValidForCommunication (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);


            /**
             * @see I_DatatypeChecks::errorIfArrayNotValidForCommunication.
             */
            GTI_ANALYSIS_RETURN errorIfArrayNotValidForCommunication (
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType* datatypes,
                    int count);


            /**
             * @see I_DatatypeChecks::errorIfNotValidForCreate.
             */
            GTI_ANALYSIS_RETURN errorIfNotValidForCreate(
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);

            /**
             * @see I_DatatypeChecks::errorIfArrayNotValidForCreate.
             */
            GTI_ANALYSIS_RETURN errorIfArrayNotValidForCreate(
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType* datatypes,
                    int count);

            /**
             * @see I_DatatypeChecks::errorIfNotValidForCommit.
             */
            GTI_ANALYSIS_RETURN errorIfNotValidForCommit(
                    MustParallelId pId,
                    MustLocationId lId,
                    int aId,
                    MustDatatypeType datatype);

                    
    protected:
            I_ParallelIdAnalysis* myPIdMod;
            I_CreateMessage* myLogger;
            I_ArgumentAnalysis* myArgMod;
            I_DatatypeTrack* myDatMod;
    };
} /*namespace MUST*/

#endif /*DATATYPECHECKS_H*/
