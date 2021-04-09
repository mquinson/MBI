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
 * @file I_Comm.h
 *       @see I_Comm.
 *
 *  @date 23.06.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "MustTypes.h"
#include "MustEnums.h"
#include "BaseIds.h"

#include "I_GroupTable.h"
#include "I_Destructable.h"

#include <sstream>
#include <list>

#ifndef I_COMM_H
#define I_COMM_H

namespace must
{
    /**
     * Interface for storage and accessing Information
     * on a comm as defined in MPI.
     */
    class I_Comm
    {
    public:

        /*
         * Basic information for a communicator handle.
         */
        virtual bool isNull (void) = 0; /**< True if this is MPI_COMM_NULL, isKnown=true in that case, the other pieces of information are not set. */
        virtual bool isPredefined (void) = 0; /**< True if this type is predefined and not MPI_COMM_NULL.*/

        //Only for user communicators (isKnown && !isNull && !isPredefined)
        virtual bool isCartesian (void) = 0; /**< True if this is a cartesian communicator.*/
        virtual bool isGraph (void) = 0; /**< True if this is a graph communicator.*/
        virtual bool isIntercomm (void) = 0; /**< True if this is an inter-communicator.*/

        //Only for user and predefined comms (isKnown && !isNull)
        virtual must::I_GroupTable *getGroup (void) = 0; /**< The communicators group, for intra comms the full group, for inter communicators the local group.*/

        //Only for inter-communicators
        virtual must::I_GroupTable *getRemoteGroup (void) = 0; /**< The remote group of the intercommunicator.*/

        //For all known and non-null communicators
        virtual unsigned long long getContextId (void) = 0; /**< The id that determines whether to communicator handles with equal groups actually refer to the same communicator or not.*/

        //Provide a context ID for whole communicator
        //Needs to be called by all ranks of the communicator
        virtual unsigned long long getNextContextId (void) = 0; /**< The id that determines whether to communicator handles with equal groups actually refer to the same communicator or not.*/

        /*
         * History information.
         */
        virtual MustParallelId getCreationPId (void) = 0; /**< For persistent comms information for call that created the comm, otherwise not set.*/
        virtual MustLocationId getCreationLId (void) = 0; /**< For persistent comms information for call that created the comm, otherwise not set.*/

        /*
         * Extra information for both cartesian and graph communicators.
         */
        virtual bool getReorder (void) = 0; /**< True if reordering was allowed for this communicator.*/

        /*
         * Extra information for cartesian communicators.
         *
         * Memory in this must not be freed it is still managed
         * by I_Comm.
         */
        virtual int getNdims (void) = 0; /**< Number of dimensions in the cart comm.*/
        virtual int* getDims (void) = 0; /**< Array of size ndims that specifies the size of each dimension.*/
        virtual bool* getPeriods (void) = 0; /**< Array of boolean values, value in index i specifies whether dimension i has a wrap around connection.*/

        /**
         * Extra information for graph communicators.
         *
         * Memory in this must not be freed it is still managed
         * by I_Comm.
         */
        virtual int getNnodes (void) = 0; /**< Number of nodes in the graph.*/
        virtual int* getIndices (void) = 0; /**< Array of node degrees, see MPI standard MPI_Graph_create for details.*/
        virtual int* getEdges (void) = 0; /**< Array of graph edges, see MPI standard MPI_Graph_create for details .*/

        /**
         * For communicators that are predefined and not MPI_COMM_NULL,
         * returns an enumeration that identifies the name of the
         * predefined communicator.
         *
         * @return value of predefined comm enumeration.
         */
        virtual MustMpiCommPredefined getPredefinedInfo (void) = 0;

        /**
         * If this is a predefined handle, returns the textual name of the
         * predefined MPI handle it represents.
         * @return name of handle.
         */
        virtual std::string getPredefinedName (void) = 0;

        /**
         * Returns true if this communicator is equal to the given one.
         * (Refer to the same communication group with equal context id.)
         * Returns false otherwise.
         *
         * @param other communicator to compare to.
         */
        virtual bool compareComms (
                I_Comm* other) = 0;

        /**
         * Returns true if this communicator is equal to the given one.
         * (Refer to the same communication group with equal context id.)
         * Returns false otherwise. This is the 'operator version' of
         * compareComms().
         *
         * @param other communicator to compare to.
         */
        virtual bool operator == (I_Comm &other) = 0;

        /**
         * Returns false if this communicator is equal to the given one.
         * (Refer to the same communication group with equal context id.)
         * Returns true otherwise. This is the 'operator version' of
         * the opposite of compareComms().
         *
         * @param other communicator to compare to.
         */
        virtual bool operator != (I_Comm &other) = 0;

        /**
         * Returns true if the given rank in this communicator is reachable by this
         * TBON node.
         * @param rank in comm to check for reachability.
         * @return true if reachable, false otherwise.
         */
        virtual bool isRankReachable (
                int rank) = 0;

        /**
         * Prints information for a specified comm.
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

    }; /*class I_Comm*/

    /**
     * Interface for storage and accessing Information
     * on a comm as defined in MPI. This is the persistent
     * version of the interface. The user needs to call I_CommPersistent::erase
     * when he is finished with it.
     */
    class I_CommPersistent : public I_Comm, public virtual I_Destructable
    {
    };/*class I_CommPersistent*/

}/*namespace must*/

#endif /*I_COMM_H*/

