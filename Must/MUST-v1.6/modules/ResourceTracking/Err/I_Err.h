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
 * @file I_Err.h
 *       @see I_Err.
 *
 *  @date 23.06.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "MustEnums.h"
#include "BaseIds.h"
#include <string>
#include <list>

#include "I_Destructable.h"

#ifndef I_ERR_H
#define I_ERR_H

namespace must
{
   /**
     * Interface for storage and accessing Information
     * on a err as defined in MPI.
     */
    class I_Err
    {
    public:

        /*
         * Basic information for an err handle.
         */
        virtual bool isNull (void) = 0; /**< True if this is MPI_ERRHANDLER_NULL, isKnown=true in that case, the other pieces of information are not set. */
        virtual bool isPredefined (void) = 0; /**< True if this is a predefined err.*/

        virtual MustParallelId getCreationPId (void) = 0; /**< For persistent errs information for call that created the err, otherwise not set.*/
        virtual MustLocationId getCreationLId (void) = 0; /**< For persistent errs information for call that created the err, otherwise not set.*/

        /**
         * For errorhandlers that are predefined and not MPI_ERRHANDLER_NULL,
         * returns an enumeration that identifies the name of the
         * predefined errorhandler.
         *
         * @return value of predefined errhandler enumeration.
         */
        virtual MustMpiErrPredefined getPredefinedInfo (void) = 0;

        /**
         * If this is a predefined handle, returns the textual name of the
         * predefined MPI handle it represents.
         * @return name of handle.
         */
        virtual std::string getPredefinedName (void) = 0;

        /**
         * Prints information for a specified err.
         * Designed for printing in a style that suits the usage
         * of CreateMessage.
         *
         * @param out stream to use for output.
         * @param pReferences current references to which any additional references for the new handle will be added.
         * @return true if successful.
         */
        virtual bool printInfo (
                std::stringstream &out,
                std::list<std::pair<MustParallelId,MustLocationId> > *pReferences) = 0;

    };/*class I_Err*/

    /**
     * Interface for storage and accessing Information
     * on a err as defined in MPI. This is the persistent
     * version of the interface. The user needs to call I_ErrPersistent::erase
     * when he is finished with it.
     */
    class I_ErrPersistent : public I_Err, public virtual I_Destructable
    {
    };/*class I_ErrPersistent*/

}/*namespace must*/

#endif /*I_ERR_H*/

