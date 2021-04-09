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
 * @file I_Keyval.h
 *       @see I_Keyval.
 *
 *  @date 23.06.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "MustEnums.h"
#include "BaseIds.h"
#include <string>
#include <list>
#include <iostream>

#include "I_Destructable.h"

#ifndef I_KEYVAL_H
#define I_KEYVAL_H

namespace must
{
    /**
     * Interface for storage and accessing Information
     * on a keyval as defined in MPI.
     */
    class I_Keyval
    {
    public:

        /*
         * Basic information for an keyval handle.
         */
        virtual bool isNull (void) = 0; /**< True if this is MPI_KEYVAL_INVALID, isKnown=true in that case, the other pieces of information are not set. */
        virtual bool isPredefined (void) = 0; /**< True if this is a predefined keyval.*/

        /*
         * History information.
         */
        virtual MustParallelId getCreationPId (void) = 0; /**< For persistent keyvals information for call that created the keyval, otherwise not set.*/
        virtual MustLocationId getCreationLId (void) = 0; /**< For persistent keyvals information for call that created the keyval, otherwise not set.*/

        /**
         * For keyvalues that are predefined and not MPI_KEYVAL_INVALID,
         * returns an enumeration that identifies the name of the
         * predefined keyvalue.
         *
         * @return value of predefined keyval enumeration.
         */
        virtual MustMpiKeyvalPredefined getPredefinedInfo (void) = 0;

        /**
         * If this is a predefined handle, returns the textual name of the
         * predefined MPI handle it represents.
         * @return name of handle.
         */
        virtual std::string getPredefinedName (void) = 0;

        /**
         * Prints information for a specified keyval.
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

    };/*class I_Keyval*/

    /**
     * Interface for storage and accessing Information
     * on a keyval as defined in MPI. This is the persistent
     * version of the interface. The user needs to call I_KeyvalPersistent::erase
     * when he is finished with it.
     */
    class I_KeyvalPersistent : public I_Keyval, public virtual I_Destructable
    {
    };/*class I_KeyvalPersistent*/

}/*namespace must*/

#endif /*I_KEYVAL_H*/

