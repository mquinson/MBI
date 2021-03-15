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

#include "I_Comm.h"
#include "HandleInfoBase.h"

#ifndef COMM_H
#define COMM_H

namespace must
{
    /**
     * Implementation of I_Comm (and I_CommPersistent).
     */
    class Comm : public I_CommPersistent, public HandleInfoBase
    {
    public:
        /**
         * Constructor.
         * Initializes as a MPI_COMM_NULL info.
         */
        Comm ();

        /**
         * Constructor.
         * Initializes as a valid comm.
         */
        Comm (int *pReachableBegin, int *pReachableEnd);

        /**
         * Destructor.
         */
        ~Comm ();

        bool isNull (void); /**< @see I_Comm::isNull.*/
        bool isPredefined (void); /**< @see I_Comm::isPredefined.*/
        bool isCartesian (void); /**< @see I_Comm::isCartesian.*/
        bool isGraph (void); /**< @see I_Comm::isGraph.*/
        bool isIntercomm (void); /**< @see I_Comm::isIntercomm.*/
        I_GroupTable* getGroup (void); /**< @see I_Comm::group.*/
        I_GroupTable* getRemoteGroup (void); /**< @see I_Comm::getRemoteGroup.*/
        unsigned long long getContextId (void); /**< @see I_Comm::contextId.*/
        unsigned long long getNextContextId (void); /**< @see I_Comm::getNextContextId.*/
        MustParallelId getCreationPId (void); /**< @see I_Comm::creationPId.*/
        MustLocationId getCreationLId (void); /**< @see I_Comm::creationLId.*/
        bool getReorder (void); /**< @see I_Comm::reorder.*/
        int getNdims (void); /**< @see I_Comm::ndims.*/
        int* getDims (void); /**< @see I_Comm::dims.*/
        bool* getPeriods (void); /**< @see I_Comm::periods.*/
        int getNnodes (void); /**< @see I_Comm::nnodes.*/
        int* getIndices (void); /**< @see I_Comm::indices.*/
        int* getEdges (void); /**< @see I_Comm::edges.*/
        MustMpiCommPredefined getPredefinedInfo (void); /**< @see I_Comm::getPredefinedInfo.*/
        std::string getPredefinedName (void); /**< @see I_Comm::getPredefinedName.*/
        bool compareComms (I_Comm* other); /**< @see I_Comm::compareComms.*/
        bool operator == (I_Comm &other); /**< @see I_Comm::operator==.*/
        bool operator != (I_Comm &other); /**< @see I_Comm::operator!=.*/
        bool isRankReachable (int rank); /**< @see I_Comm::isRankReachable.*/
        bool printInfo (
                std::stringstream &out,
                std::list<std::pair<MustParallelId,MustLocationId> > *pReferences); /**< @see I_Comm::printInfo.*/

        std::string getResourceName (void); /**< @see HandleInfoBase::getResourceName.*/

    public:

        bool myIsNull;
        bool myIsPredefined;
        MustMpiCommPredefined myPredefined;
        std::string myPredefinedName;

        //Only for user communicators (isKnown && !isNull && !isPredefined)
        bool myIsCartesian;
        bool myIsGraph;
        bool myIsIntercomm;

        //Only for user and predefined comms (isKnown && !isNull)
        /**
         * Identificator that is used to compare comms:
         * (IntraComms:) comm1 == comm2 iff group1 == group2 ^ context1 == context2
         * (InterComms:) comm1 == comm2 iff ((group1 == group2 ^ remoteGroup1 == remoteGroup2) v (group1 == remoteGroup2 ^ remoteGroup1 == group2)) ^ context1 == context2
         * (comparing groups can be done by comparing their pointers, as each distinct group will only be
         * created once)
         *
         * @TODO this is crude, unsatisfactory, and requires to store one MPI_COMM_WORLD info for each rank! This should be modified to a pointer to a parent or something to compare comms based on their ancestors must be refined.
         */
        unsigned long long myContextId;
        /**
         * Identificator that is used to compute the context id of a new comm
         * during its creation. The new communicator receives nextContextId*N
         * where N is some number (128 currently) as nextContextId and
         * nextContextId as its actual context id. With this a contextId can be
         * computed without any inter-process communication, though conflicts
         * or overflows may appear for extensive communicator creation.
         */
        unsigned long long myNextContextId; /**< Value of the next context id for comm creating calls on this communicator.*/
        I_GroupTable *myGroup;

        //Only for inter-communicators
        I_GroupTable *myRemoteGroup; /**< The remote group of the intercommunicator.*/

        //Only for user defined comms
        MustParallelId myCreationPId;
        MustLocationId myCreationLId;

        //For cartesian or graph comms
        bool myReorder;

        //Only for cartesian comms
        int myNdims;
        int *myDims;
        bool *myPeriods;

        //Only for graph comms
        int myNnodes;
        int *myIndices;
        int *myEdges;

        //Information on processes reachable on this node
        int *myPReachableBegin; /**< Pointer to storage for beginning MPI_COMM_WORLD rank of reachable processes.*/
        int *myPReachableEnd; /**< Pointer to storage for last MPI_COMM_WORLD rank of reachable processes.*/

    };/*class Comm*/
}/*namespace must*/

#endif /*COMM_H*/

