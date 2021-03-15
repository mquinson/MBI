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
 * @file CollectiveOp.cpp
 *       @see must::CollectiveOp.
 *
 *  @date 30.07.2011
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "CollectiveOp.h"

using namespace must;

//=============================
// Constructor
//=============================
CollectiveOp::CollectiveOp (
        CollectiveMatch* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (false),
  myIsReceiveTransfer (false),
  myIsToOne (false),
  myCollId (collId),
  myComm (comm),
  myCount (0),
  myCounts (NULL),
  myType (NULL),
  myTypes (NULL),
  myOp (NULL),
  myDestSource (0),
  myRank (matcher->myPIdMod->getInfoForId(pId).rank)
{
    initializeCommSize();
}

//=============================
// Constructor
//=============================
CollectiveOp::CollectiveOp (
        CollectiveMatch* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        bool isSend,
        int count,
        I_DatatypePersistent* type,
        I_OpPersistent *op,
        int destSource
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (isSend),
  myIsReceiveTransfer (!isSend),
  myIsToOne (true),
  myCollId (collId),
  myComm (comm),
  myCount (count),
  myCounts (NULL),
  myType (type),
  myTypes (NULL),
  myOp (op),
  myDestSource (destSource),
  myRank (matcher->myPIdMod->getInfoForId(pId).rank)
{
    initializeCommSize();
}

//=============================
// Constructor
//=============================
CollectiveOp::CollectiveOp (
        CollectiveMatch* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        bool isSend,
        int count,
        I_DatatypePersistent* type,
        I_OpPersistent *op
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (isSend),
  myIsReceiveTransfer (!isSend),
  myIsToOne (false),
  myCollId (collId),
  myComm (comm),
  myCount (count),
  myCounts (NULL),
  myType (type),
  myTypes (NULL),
  myOp (op),
  myDestSource (0),
  myRank (matcher->myPIdMod->getInfoForId(pId).rank)
{
    initializeCommSize();
}

//=============================
// Constructor
//=============================
CollectiveOp::CollectiveOp (
        CollectiveMatch* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        bool isSend,
        int* counts,
        I_DatatypePersistent* type,
        I_OpPersistent *op
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (isSend),
  myIsReceiveTransfer (!isSend),
  myIsToOne (false),
  myCollId (collId),
  myComm (comm),
  myCount (0),
  myCounts (counts),
  myType (type),
  myTypes (NULL),
  myOp (op),
  myDestSource (0),
  myRank (matcher->myPIdMod->getInfoForId(pId).rank)
{
    initializeCommSize();
}

//=============================
// Constructor
//=============================
CollectiveOp::CollectiveOp (
        CollectiveMatch* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        bool isSend,
        int* counts,
        I_DatatypePersistent** types,
        I_OpPersistent *op
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (isSend),
  myIsReceiveTransfer (!isSend),
  myIsToOne (false),
  myCollId (collId),
  myComm (comm),
  myCount (0),
  myCounts (counts),
  myType (NULL),
  myTypes (types),
  myOp (op),
  myDestSource (0),
  myRank (matcher->myPIdMod->getInfoForId(pId).rank)
{
    initializeCommSize();
}

//=============================
// Destructor
//=============================
CollectiveOp::~CollectiveOp (void)
{
    myMatcher = NULL;

    if (myComm) myComm->erase();
    myComm = NULL;

    if (myCounts)
        delete [] myCounts;

    if (myType) myType->erase();
    myType = NULL;

    if (myTypes)
    {
      for (int i = 0; i < myCommSize; i++)
      {
          if (myTypes[i]) myTypes[i]->erase();
      }
      delete [] myTypes;
      myTypes=NULL;
    }

    if (myOp) myOp->erase();
    myOp = NULL;
}

//=============================
// initializeCommSize
//=============================
void CollectiveOp::initializeCommSize (void)
{
    //For intra comms its the group size, for inter comms its the sum of the two group sizes!
    myCommSize = myComm->getGroup()->getSize();

    if (myComm->isIntercomm())
        myCommSize += myComm->getRemoteGroup()->getSize();
}

//=============================
// process
//=============================
PROCESSING_RETURN CollectiveOp::process (int rank)
{
    //==Process
    myMatcher->addCollectiveTransfer (this);

    return PROCESSING_SUCCESS;
}

//=============================
// print
//=============================
GTI_RETURN CollectiveOp::print (std::ostream &out)
{
    out << "Coll ";

    if (myIsSendTransfer)
        out << "send";
    else if (myIsReceiveTransfer)
        out << "receive";
    else
        out << "no";

    out << " transfer";

    if (myIsToOne)
        out << " directed to " << myDestSource << "(MPI_COMM_WORLD)";
    else
        out << " directed to all tasks";

    out << " collId=" << myCollId;

    return GTI_SUCCESS;
}

//=============================
// getComm
//=============================
I_Comm* CollectiveOp::getComm (void)
{
    return myComm;
}

//=============================
// getCommCopy
//=============================
I_CommPersistent* CollectiveOp::getCommCopy (void)
{
    myComm->copy();
    return myComm;
}

//=============================
// getPersistentComm
//=============================
I_CommPersistent* CollectiveOp::getPersistentComm (void)
{
    return myComm;
}

//=============================
// getCollId
//=============================
MustCollCommType CollectiveOp::getCollId (void)
{
    return myCollId;
}

//=============================
// getIssuerRank
//=============================
int CollectiveOp::getIssuerRank (void)
{
    return myRank;
}

//=============================
// getCommSize
//=============================
int CollectiveOp::getCommSize (void)
{
    return myCommSize;
}

//=============================
// getRoot
//=============================
int CollectiveOp::getRoot (void)
{
    if (myIsToOne)
        return myDestSource;

    //If this is a N transfer the root must be this rank itself (see hasRoot) or its an erroneous
    //query
    return myRank;
}

//=============================
// isToOne
//=============================
bool CollectiveOp::isToOne (void)
{
    return myIsToOne;
}

//=============================
// hasRoot
//=============================
bool CollectiveOp::hasRoot (void)
{
    if (myIsToOne)
        return true;

    if (myIsSendTransfer &&
            (       myCollId == MUST_COLL_BCAST ||
                    myCollId == MUST_COLL_SCATTER ||
                    myCollId == MUST_COLL_SCATTERV
            )
        )
        return true;

    if (myIsReceiveTransfer &&
            (myCollId == MUST_COLL_GATHER ||
             myCollId == MUST_COLL_GATHERV ||
             myCollId == MUST_COLL_REDUCE)
        )
        return true;

    return false;
}

//=============================
// isSendTransfer
//=============================
bool CollectiveOp::isSendTransfer(void)
{
    return myIsSendTransfer;
}

//=============================
// isReceiveTransfer
//=============================
bool CollectiveOp::isReceiveTransfer(void)
{
    return myIsReceiveTransfer;
}

//=============================
// isNoTransfer
//=============================
bool CollectiveOp::isNoTransfer(void)
{
    return (!myIsSendTransfer && !myIsReceiveTransfer);
}

//=============================
// hasOp
//=============================
bool CollectiveOp::hasOp (void)
{
    return (myOp != NULL);
}

//=============================
// getOp
//=============================
I_Op* CollectiveOp::getOp (void)
{
    return myOp;
}

//=============================
// getPId
//=============================
MustParallelId CollectiveOp::getPId (void)
{
    return myPId;
}

//=============================
// requiresSecondOp
//=============================
bool CollectiveOp::requiresSecondOp (void)
{
    //No transfer ops need no second op
    if (!myIsSendTransfer && !myIsReceiveTransfer)
        return false;

    //Receive ops never require a second op as sends are always propagated before receives (if both are needed)
    if (myIsReceiveTransfer)
        return false;

    //If this is a send operation we need to decide based on the collectiv ID
    switch (myCollId)
    {
    case MUST_COLL_GATHER:
    case MUST_COLL_GATHERV:
    case MUST_COLL_REDUCE:
        if (myRank == myDestSource)
            return true;
        else return false;
    case MUST_COLL_BCAST:
        return false; //Only the root of the BCast sends, all other only receive
    default:
        //For all other cases
        return true;
    }

    return false;
}

//=============================
// printCollectivemismatch
//=============================
bool CollectiveOp::printCollectiveMismatch (CollectiveOp *other)
{
    //Sanity check
    if (myCollId == other->myCollId)
        return true;

    std::list <std::pair <MustParallelId, MustLocationId> > references;
    std::stringstream stream;

    stream
        << "A collective mismatch occured (The application executes two different collective calls on the same communicator)! "
        << "The collective operation that does not matches this operation was executed at reference 1.";
    references.push_back(std::make_pair (other->myPId, other->myLId));

    stream << " (Information on communicator: ";
    myComm->printInfo(stream,&references);
    stream << ")";
    stream << std::endl << "Note that collective matching was disabled as a result, collectives won't be analysed for their correctness or blocking state anymore. You should solve this issue and rerun your application with MUST.";

    myMatcher->myLogger->createMessage (
            MUST_ERROR_COLLECTIVE_CALL_MISSMATCH,
            myPId,
            myLId,
            MustErrorMessage,
            stream.str(),
            references);

    return true;
}

//=============================
// printRootMismatch
//=============================
bool CollectiveOp::printRootMismatch (CollectiveOp *other)
{
    //Sanity check
    if (!hasRoot() || !other->hasRoot() || getRoot() == other->getRoot())
        return true;

    std::list <std::pair <MustParallelId, MustLocationId> > references;
    std::stringstream stream;

    stream
    << "Two collective operations that use a root process specified conflicting roots! "
    << "This collective uses rank " << getRoot() << " as root (As rank in MPI_COMM_WORLD)."
    << "The conflicting operation uses rank " << other->getRoot() << " as root (rank in MPI_COMM_WORLD) and was executed at reference 1.";
    references.push_back(std::make_pair (other->myPId, other->myLId));

    stream << " (Information on communicator: ";
    myComm->printInfo(stream,&references);
    stream << ")";
    stream << std::endl << "Note that collective matching was disabled as a result, collectives won't be analysed for their correctness or blocking state anymore. You should solve this issue and rerun your application with MUST.";

    myMatcher->myLogger->createMessage (
            MUST_ERROR_COLLECTIVE_ROOT_MISSMATCH,
            myPId,
            myLId,
            MustErrorMessage,
            stream.str(),
            references);

    return true;
}

//=============================
// printOpMismatch
//=============================
bool CollectiveOp::printOpMismatch (CollectiveOp *other)
{
    //Sanity check
    if (    !hasOp() ||
            !other->hasOp() ||
            (!myOp->isPredefined() && !other->myOp->isPredefined()) ||
            (myOp->isPredefined() && !other->myOp->isPredefined()) ||
            (!myOp->isPredefined() && other->myOp->isPredefined()) ||
            (myOp->getPredefinedInfo() == other->myOp->getPredefinedInfo())
            )
        return true;

    std::list <std::pair <MustParallelId, MustLocationId> > references;
    std::stringstream stream;

    stream
    << "Two collective calls that use an operation specified conflicting operations! "
    << "This rank uses the operation: ";
    myOp->printInfo(stream, &references);

    references.push_back(std::make_pair (other->myPId, other->myLId));
    stream
    << ". "
    << "The conflicting call that was executed at reference " << references.size() << " uses the operation: ";
    other->myOp->printInfo(stream, &references);

    stream
    << ". "
    << "(Information on communicator: ";
    myComm->printInfo(stream,&references);
    stream << ")";
    stream << std::endl << "Note that collective matching was disabled as a result, collectives won't be analysed for their correctness or blocking state anymore. You should solve this issue and rerun your application with MUST.";

    myMatcher->myLogger->createMessage (
            MUST_ERROR_COLLECTIVE_OP_MISSMATCH,
            myPId,
            myLId,
            MustErrorMessage,
            stream.str(),
            references);

    return true;
}

//=============================
// validateTypeMatch
//=============================
bool CollectiveOp::validateTypeMatch (CollectiveOp* other)
{
    int                 sendCount, receiveCount;
    I_Datatype     *sendType, *receiveType;
    CollectiveOp  *send, *receive;

    //==1) Sort into send Receive
    if (myIsSendTransfer)
    {
        send = this;
        receive = other;
    }
    else
    {
        send = other;
        receive = this;
    }

    //==2) Determine send/recv count and type
    //SEND
    int receiveCommRank;

    if (send->myCounts || send->myTypes)
    {
        if (!send->myComm->getGroup()->containsWorldRank(receive->myRank, &receiveCommRank))
        {
            assert (0);//The issuer rank of the receive must be in the comm
            return false;
        }
    }

    if (send->myCounts)
        sendCount = send->myCounts[receiveCommRank];
    else
        sendCount = send->myCount;

    if (send->myTypes)
        sendType = send->myTypes[receiveCommRank];
    else
        sendType = send->myType;

    //RECEIVE
    int sendCommRank;

    if (receive->myCounts || receive->myTypes)
    {
        if (!receive->myComm->getGroup()->containsWorldRank(send->myRank, &sendCommRank))
        {
            assert (0);//The issuer rank of the receive must be in the comm
            return false;
        }
    }

    if (receive->myCounts)
        receiveCount = receive->myCounts[sendCommRank];
    else
        receiveCount = receive->myCount;

    if (receive->myTypes)
        receiveType = receive->myTypes[sendCommRank];
    else
        receiveType = receive->myType;

    //==3) Match the types
    MustMessageIdNames ret = MUST_ERROR_TYPEMATCH_INTERNAL_NOTYPE;
    MustAddressType pos = 0;
    ret = sendType->isEqualB(sendCount, receiveType, receiveCount, &pos);

    std::stringstream stream;
    std::list <std::pair <MustParallelId, MustLocationId> > references;

    switch (ret)
    {
    case MUST_ERROR_TYPEMATCH_MISSMATCH:
        stream
        << "Two collective calls cause a type mismatch! This call sends data to the call in reference 1.";
        references.push_back(std::make_pair(receive->myPId, receive->myLId));

        stream
        << " The mismatch occurs at ";

        sendType->printDatatypeLongPos(stream, pos);

        stream << " in the send type and at ";

        receiveType->printDatatypeLongPos(stream, pos);

        stream << " in the receive type (consult the MUST manual for a detailed description of datatype positions).";

        break;
    case MUST_ERROR_TYPEMATCH_MISSMATCH_BYTE:
        //TODO
        assert (0);
        break;
    case MUST_ERROR_TYPEMATCH_INTERNAL_NOTYPE:
    case MUST_ERROR_TYPEMATCH_INTERNAL_TYPESIG:
        //Both should be catched by different types of checks
        return true;
    case MUST_ERROR_TYPEMATCH_LENGTH:
        stream
        << "Two collective calls use (datatype,count) pairs that span type signatures of different length!"
        << " Each send and receive transfer of a collective call must use equal type signatures (I.e. same types with potentially different displacements)."
        << " This is the sending operation and the receiving operation is issued at reference 1.";
        references.push_back(std::make_pair(receive->myPId, receive->myLId));

        if (sendType->getSize() * sendCount < receiveType->getSize() * receiveCount)
        {
            stream
            << " The first element of the receive that did not fit into the send operation is at ";
            receiveType->printDatatypeLongPos(stream, pos);
            stream << " in the receive type (consult the MUST manual for a detailed description of datatype positions).";
        }
        else
        {
            stream
            << " The first element of the send that did not fit into the receive operation is at ";
            sendType->printDatatypeLongPos(stream, pos);
            stream << " in the send type (consult the MUST manual for a detailed description of datatype positions).";
        }
        break;
    default:
        return true;
    }

    stream << " (Information on communicator: ";
    myComm->printInfo(stream,&references);
    stream << ")";

    stream
    << " (Information on send transfer of count "
    << sendCount
    << " with type:";

    sendType->printInfo(stream, &references);
    stream << ")";

    stream
    << " (Information on receive of count "
    << receiveCount
    << " with type:";

    receiveType->printInfo(stream, &references);

    myMatcher->myLogger->createMessage (
            ret,
            send->myPId,
            send->myLId,
            MustErrorMessage,
            stream.str(),
            references);

    return false;
}

//=============================
// hasMultipleTypes
//=============================
bool CollectiveOp::hasMultipleTypes (void)
{
    if (myTypes)
        return true;
    return false;
}

//=============================
// hasMultipleCounts
//=============================
bool CollectiveOp::hasMultipleCounts (void)
{
    if (myCounts)
        return true;
    return false;
}

//=============================
// collectiveUsesMultipleCounts
//=============================
bool CollectiveOp::collectiveUsesMultipleCounts (void)
{
    if (myCollId == MUST_COLL_GATHERV ||
            myCollId == MUST_COLL_SCATTERV ||
            myCollId == MUST_COLL_ALLGATHERV ||
            myCollId == MUST_COLL_ALLTOALLV ||
            myCollId == MUST_COLL_ALLTOALLW ||
            myCollId == MUST_COLL_REDUCE_SCATTER)
        return true;

    return false;
}

//=============================
// copy
//=============================
CollectiveOp* CollectiveOp::copy (void)
{
    return new CollectiveOp (this);
}

//=============================
// CollectiveOp (from existing op)
//=============================
CollectiveOp::CollectiveOp (CollectiveOp* other)
{
    myMatcher = other->myMatcher;
    myPId = other->myPId;
    myLId = other->myLId;
    myIsSendTransfer = other->myIsSendTransfer;
    myIsReceiveTransfer = other->myIsReceiveTransfer;
    myIsToOne = other->myIsToOne;
    myCollId = other->myCollId;
    myComm = other->myComm;
    if (myComm) myComm->copy();
    myCount = other->myCount;
    myCommSize = other->myCommSize;

    if (other->myCounts)
    {
        myCounts = new int[myCommSize];
        for (int i = 0; i < myCommSize; i++)
            myCounts[i] = other->myCounts[i];
    }
    else
    {
        myCounts = NULL;
    }

    myType = other->myType;
    if (myType) myType->copy();

    if (other->myTypes)
    {
        myTypes = new I_DatatypePersistent* [myCommSize];
        for (int i = 0; i < myCommSize; i++)
        {
            myTypes[i] = other->myTypes[i];
            if (myTypes[i]) myTypes[i]->copy();
        }
    }
    else
    {
        myTypes = NULL;
    }

    myOp = other->myOp;
    if (myOp) myOp->copy();

    myDestSource = other->myDestSource;
    myRank = other->myRank;
}

//=============================
// copyQueuedOp
//=============================
I_Operation* CollectiveOp::copyQueuedOp (void)
{
    return copy();
}

/*EOF*/
