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
 * @file DCollectiveTypeMatchInfo.h
 *       @see must::DCollectiveTypeMatchInfo.
 *
 *  @date 29.05.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#ifndef DCOLLECTIVETYPEMATCHINFO_H
#define DCOLLECTIVETYPEMATCHINFO_H

#include "I_Datatype.h"
#include "I_Comm.h"
#include "BaseIds.h"

namespace must
{
    /**
     * Class to hold information for intra layer type matching.
     */
    class DCollectiveTypeMatchInfo
    {
    public:
        /**
         * Constructor.
         * @param pId of origin context.
         * @param lId of origin context.
         * @param comm in use (managment of this is transfered to the new object).
         * @param type in use (managment of this is transfered to the new object).
         * @param numCounts number of type match pairs in this info.
         * @param counts for typematching (needs to be copied).
         * @param firstRank start rank for the counts.
         * @param collectiveNumber of this collective in its communicator.
         * @param collId collective operation type of this type match information.
         */
        DCollectiveTypeMatchInfo (
                int rank,
                MustParallelId pId,
                MustLocationId lId,
                I_CommPersistent *comm,
                I_DatatypePersistent *type,
                int numCounts,
                int *counts,
                int firstRank,
                int collectiveNumber,
                MustCollCommType collId);

        /**
         * Constructor.
         * @param pId of origin context.
         * @param lId of origin context.
         * @param comm in use (managment of this is transfered to the new object).
         * @param types in use (managment of the array and its contents is transfered to the new object).
         * @param numCounts number of type match pairs in this info.
         * @param counts for typematching (needs to be copied).
         * @param startRank for the counts.
         * @param collectiveNumber of this collective in its communicator.
         * @param collId collective operation type of this type match information.
         */
        DCollectiveTypeMatchInfo (
                int rank,
                MustParallelId pId,
                MustLocationId lId,
                I_CommPersistent *comm,
                int numCounts,
                I_DatatypePersistent **types,
                int *counts,
                int firstRank,
                int collectiveNumber,
                MustCollCommType collId);

        /**
         * Destructor.
         */
        ~DCollectiveTypeMatchInfo ();

        /**
         * Returns the wave number associated with this type match info.
         */
        int getWaveNumber (void);

        /**
         * Returns the collective id of this type match info.
         */
        MustCollCommType getCollId (void);

        /**
         * Returns the size of the count/rank[/types] arrays.
         */
        int getNumCounts (void);

        /**
         * Provides access to the counts array, memory is still managed
         * by this object.
         */
        int* getCounts (void);

        /**
         * Returns rank associated with the first count.
         */
        int getFirstRank (void);

        /**
         * True if this type match information uses multiple types.
         */
        bool hasTypes (void);

        /**
         * Provides access to the types array, memory is still managed
         * by this object. Persistent types are still managed by this
         * object.
         */
        I_DatatypePersistent** getTypes (void);

        /**
         * Provides access to the datatype used by this type match info
         * (if a single type, i.e. hasTypes returns false).
         * Persistent type is still managed by this object.
         */
        I_DatatypePersistent* getType (void);

        /**
         * Returns PId of this type match info.
         */
        MustParallelId getPId (void);

        /**
         * Returns LId of this type match info.
         */
        MustLocationId getLId (void);

        /**
         * Returns the rank from which this info is.
         */
        int getRank (void);

    protected:

        int myRank;
        MustParallelId myPId;
        MustLocationId myLId;

        I_CommPersistent *myComm;
        I_DatatypePersistent *myType;
        I_DatatypePersistent **myTypes;

        int myNumCounts;
        int *myCounts;
        int myFirstRank;
        int myCollectiveNumber;
        MustCollCommType myCollId;
    };
} /*namespace must*/

#endif /*DCOLLECTIVETYPEMATCHINFO_H*/
