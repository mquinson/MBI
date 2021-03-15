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
 * @file Op.h
 *       @see Op.
 *
 *  @date 15.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Op.h"
#include "HandleInfoBase.h"

#include <string>

#ifndef OP_H
#define OP_H

namespace must
{
    /**
     * Implementation of I_Comm (and I_CommPersistent).
     */
    class Op : public I_OpPersistent, public HandleInfoBase
    {
    public:
        /**
         * Constructor.
         * Initializes as a MPI_OP_NULL info.
         */
        Op ();

        /**
         * Constructor.
         * Initializes as a predefined operation.
         */
        Op (MustMpiOpPredefined predefined, const char* predefinedName);

        /**
         * Destructor.
         */
        ~Op ();

        bool isNull (void); /**< @see I_Op::isNull. */
        bool isPredefined (void); /**< @see I_Op::isPredefined. */
        bool isCommutative (void); /**< @see I_Op::isCommutative. */
        MustParallelId getCreationPId (void); /**< @see I_Op::getCreationPId. */
        MustLocationId getCreationLId (void); /**< @see I_Op::getCreationLId. */
        MustMpiOpPredefined getPredefinedInfo (void); /**< @see I_Op::getPredefinedInfo. */
        std::string getPredefinedName (void); /**< @see I_Op::getPredefinedName. */
        bool printInfo (
                std::stringstream &out,
                std::list<std::pair<MustParallelId,MustLocationId> > *pReferences); /**< @see I_Op::printInfo. */

        std::string getResourceName (void); /**< @see HandleInfoBase::getResourceName.*/

    public:

        MustMpiOpPredefined myPredefined;
        std::string myPredefinedName;

        bool myIsNull;
        bool myIsPredefined;
        bool myIsCommutative;
        MustParallelId myCreationPId;
        MustLocationId myCreationLId;

        bool operator == (I_Op &other); /**< @see I_Op::operator ==. */
        bool operator != (I_Op &other); /**< @see I_Op::operator !=. */
    };/*class Op*/
}/*namespace must*/

#endif /*OP_H*/

