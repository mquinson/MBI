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
 * @file I_Op.h
 *       @see I_Op.
 *
 *  @date 23.06.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#include "MustEnums.h"
#include "BaseIds.h"
#include <string>
#include <list>

#include "I_Destructable.h"

#ifndef I_OP_H
#define I_OP_H

namespace must
{
    /**
     * Interface for storage and accessing Information
     * on a op as defined in MPI.
     */
    class I_Op
    {
    public:

        /*
         * Basic information for an op handle.
         */
        virtual bool isNull (void) = 0; /**< True if this is MPI_COMM_NULL, isKnown=true in that case, the other pieces of information are not set. */
        virtual bool isPredefined (void) = 0; /**< True if this is a predefined op.*/
        virtual bool isCommutative (void) = 0; /**< True if this operation is commutative, false otherwise.*/

        /*
         * History information.
         */
        virtual MustParallelId getCreationPId (void) = 0; /**< For persistent ops information for call that created the op, otherwise not set.*/
        virtual MustLocationId getCreationLId (void) = 0; /**< For persistent ops information for call that created the op, otherwise not set.*/

        /**
         * For ops that are predefined and not MPI_OP_NULL,
         * returns an enumeration that identifies the name of the
         * predefined operation.
         *
         * @return value of predefined op enumeration.
         */
        virtual MustMpiOpPredefined getPredefinedInfo (void) = 0;

        /**
         * If this is a predefined handle, returns the textual name of the
         * predefined MPI handle it represents.
         * @return name of handle.
         */
        virtual std::string getPredefinedName (void) = 0;

        /**
         * Prints information for a specified op.
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

        /**
         * Compares two persistent operations. While this works perfectly for
         * operations defined by the MPI standard, please note that we CAN NOT
         * CHECK the content of user-supplied, custom ones. These will be seen
         * as equal iff both are user-supplied operations (but no further checks).
         *
         * @param other persistent operation to compare to.
         * @return True, if equal. Otherwise false.
         */
        virtual bool operator == (I_Op &other) = 0;

        /**
         * Compares two persistent operations. While this works perfectly for
         * operations defined by the MPI standard, please note that we CAN NOT
         * CHECK the content of user-supplied, custom ones. These will be seen
         * as equal iff both are user-supplied operations (but no further checks).
         *
         * This is the opposite of ==.
         *
         * @param other persistent operation to compare to.
         * @return False, if equal. Otherwise true.
         */
        virtual bool operator != (I_Op &other) = 0;

    };/*class I_Op*/

    /**
     * Interface for storage and accessing Information
     * on an operation as defined in MPI. This is the persistent
     * version of the interface. The user needs to call I_OpPersistent::erase
     * when he is finished with it.
     */
    class I_OpPersistent : public I_Op, public virtual I_Destructable
    {
    };/*class I_OpPersistent*/

}/*namespace must*/

#endif /*I_OP_H*/

