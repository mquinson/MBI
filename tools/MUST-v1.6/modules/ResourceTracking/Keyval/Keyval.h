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
 *  @date 15.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "I_Keyval.h"
#include "HandleInfoBase.h"

#include <string>

#ifndef KEYVAL_H
#define KEYVAL_H

namespace must
{
    /**
     * Implementation of I_Comm (and I_CommPersistent).
     */
    class Keyval : public I_KeyvalPersistent, public HandleInfoBase
    {
    public:
        /**
         * Constructor.
         * Initializes as a MPI_KEYVAL_INVALID info.
         */
        Keyval ();

        /**
         * Constructor.
         * Initializes as a predefined keyvalue.
         */
        Keyval (MustMpiKeyvalPredefined predefined, std::string predefinedName);

        /**
         * Destructor.
         */
        ~Keyval ();

        bool isNull (void); /**< @see I_Keyval::.*/
        bool isPredefined (void); /**< @see I_Keyval::.*/
        MustParallelId getCreationPId (void); /**< @see I_Keyval::.*/
        MustLocationId getCreationLId (void); /**< @see I_Keyval::.*/
        MustMpiKeyvalPredefined getPredefinedInfo (void); /**< @see I_Keyval::.*/
        std::string getPredefinedName (void); /**< @see I_Keyval::.*/
        bool printInfo (
                std::stringstream &out,
                std::list<std::pair<MustParallelId,MustLocationId> > *pReferences); /**< @see I_Keyval::.*/

        std::string getResourceName (void); /**< @see HandleInfoBase::getResourceName.*/

    public:
        MustMpiKeyvalPredefined myPredefined;
        std::string myPredefinedName;

        bool myIsNull;
        bool myIsPredefined;
        MustParallelId myCreationPId;
        MustLocationId myCreationLId;

    };/*class Keyval*/
}/*namespace must*/

#endif /*KEYVAL_H*/

