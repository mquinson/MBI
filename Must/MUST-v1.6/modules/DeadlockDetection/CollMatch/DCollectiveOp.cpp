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
 * @file DCollectiveOp.cpp
 *       @see must::DCollectiveOp.
 *
 *  @date 25.04.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat, Fabian Haensel
 */

#include "DCollectiveOp.h"

#include <map>
#include <assert.h>

using namespace must;

//=============================
// Constructor
//=============================
DCollectiveOp::DCollectiveOp (
        I_DCollectiveOpProcessor* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        MustCommType commHandle,
        int numRanks,
        int fromChannel,
        bool hasRequest,
        MustRequestType request
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (false),
  myIsReceiveTransfer (false),
  myIsToOne (false),
  myCollId (collId),
  myComm (comm),
  myCommHandle (commHandle),
  myCount (0),
  myCounts (NULL),
  myType (NULL),
  myTypeHandle (0),
  myTypes (NULL),
  myTypeHandles (NULL),
  myOp (NULL),
  myOpHandle (0),
  myDestSource (0),
  myRank (matcher->pIdToRank(pId)),
  myNumRanks (numRanks),
  myFromChannel (fromChannel),
  myTS (0),
  myHasRequest (hasRequest),
  myRequest (request)
{
    initializeCommSize();
}

//=============================
// Constructor
//=============================
DCollectiveOp::DCollectiveOp (
        I_DCollectiveOpProcessor* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        MustCommType commHandle,
        bool isSend,
        int count,
        I_DatatypePersistent* type,
        MustDatatypeType typeHandle,
        I_OpPersistent *op,
        MustOpType opHandle,
        int destSource,
        int numRanks,
        int fromChannel,
        bool hasRequest,
        MustRequestType request
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (isSend),
  myIsReceiveTransfer (!isSend),
  myIsToOne (true),
  myCollId (collId),
  myComm (comm),
  myCommHandle (commHandle),
  myCount (count),
  myCounts (NULL),
  myType (type),
  myTypeHandle (typeHandle),
  myTypes (NULL),
  myTypeHandles (NULL),
  myOp (op),
  myOpHandle (opHandle),
  myDestSource (destSource),
  myRank (matcher->pIdToRank(pId)),
  myNumRanks (numRanks),
  myFromChannel (fromChannel),
  myTS (0),
  myHasRequest (hasRequest),
  myRequest (request)
{
    initializeCommSize();
}

//=============================
// Constructor
//=============================
DCollectiveOp::DCollectiveOp (
        I_DCollectiveOpProcessor* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        MustCommType commHandle,
        bool isSend,
        int count,
        I_DatatypePersistent* type,
        MustDatatypeType typeHandle,
        I_OpPersistent *op,
        MustOpType opHandle,
        int numRanks,
        int fromChannel,
        bool hasRequest,
        MustRequestType request
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (isSend),
  myIsReceiveTransfer (!isSend),
  myIsToOne (false),
  myCollId (collId),
  myComm (comm),
  myCommHandle (commHandle),
  myCount (count),
  myCounts (NULL),
  myType (type),
  myTypeHandle (typeHandle),
  myTypes (NULL),
  myTypeHandles (NULL),
  myOp (op),
  myOpHandle (opHandle),
  myDestSource (0),
  myRank (matcher->pIdToRank(pId)),
  myNumRanks (numRanks),
  myFromChannel (fromChannel),
  myTS (0),
  myHasRequest (hasRequest),
  myRequest (request)
{
    initializeCommSize();
}

//=============================
// Constructor
//=============================
DCollectiveOp::DCollectiveOp (
        I_DCollectiveOpProcessor* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        MustCommType commHandle,
        bool isSend,
        int* counts,
        I_DatatypePersistent* type,
        MustDatatypeType typeHandle,
        I_OpPersistent *op,
        MustOpType opHandle,
        int numRanks,
        int fromChannel,
        bool hasRequest,
        MustRequestType request
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (isSend),
  myIsReceiveTransfer (!isSend),
  myIsToOne (false),
  myCollId (collId),
  myComm (comm),
  myCommHandle (commHandle),
  myCount (0),
  myCounts (counts),
  myType (type),
  myTypeHandle (typeHandle),
  myTypes (NULL),
  myTypeHandles (NULL),
  myOp (op),
  myOpHandle (opHandle),
  myDestSource (0),
  myRank (matcher->pIdToRank(pId)),
  myNumRanks (numRanks),
  myFromChannel (fromChannel),
  myTS (0),
  myHasRequest (hasRequest),
  myRequest (request)
{
    initializeCommSize();
}

//=============================
// Constructor
//=============================
DCollectiveOp::DCollectiveOp (
        I_DCollectiveOpProcessor* matcher,
        MustParallelId pId,
        MustLocationId lId,
        MustCollCommType collId,
        I_CommPersistent *comm,
        MustCommType commHandle,
        bool isSend,
        int* counts,
        I_DatatypePersistent** types,
        MustDatatypeType *typeHandles,
        I_OpPersistent *op,
        MustOpType opHandle,
        int numRanks,
        int fromChannel,
        bool hasRequest,
        MustRequestType request
)
: myMatcher (matcher),
  myPId (pId),
  myLId (lId),
  myIsSendTransfer (isSend),
  myIsReceiveTransfer (!isSend),
  myIsToOne (false),
  myCollId (collId),
  myComm (comm),
  myCommHandle (commHandle),
  myCount (0),
  myCounts (counts),
  myType (NULL),
  myTypeHandle (0),
  myTypes (types),
  myTypeHandles (typeHandles),
  myOp (op),
  myOpHandle (opHandle),
  myDestSource (0),
  myRank (matcher->pIdToRank(pId)),
  myNumRanks (numRanks),
  myFromChannel (fromChannel),
  myTS (0),
  myHasRequest (hasRequest),
  myRequest (request)
{
    initializeCommSize();
}

//=============================
// Destructor
//=============================
DCollectiveOp::~DCollectiveOp (void)
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

    if (myTypeHandles)
        delete [] myTypeHandles;
    myTypeHandles = NULL;

    if (myOp) myOp->erase();
    myOp = NULL;
}

//=============================
// initializeCommSize
//=============================
void DCollectiveOp::initializeCommSize (void)
{
    //For intra comms its the group size, for inter comms its the sum of the two group sizes!
    myCommSize = myComm->getGroup()->getSize();

    if (myComm->isIntercomm())
        myCommSize += myComm->getRemoteGroup()->getSize();
}

//=============================
// process
//=============================
PROCESSING_RETURN DCollectiveOp::process (int rank)
{
    /*This should never be called at all, just an I_OperationReordering reminder*/
    assert (0);

    return PROCESSING_SUCCESS;
}

//=============================
// print
//=============================
GTI_RETURN DCollectiveOp::print (std::ostream &out)
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
I_Comm* DCollectiveOp::getComm (void)
{
    return myComm;
}

//=============================
// getCommCopy
//=============================
I_CommPersistent* DCollectiveOp::getCommCopy (void)
{
    myComm->copy();
    return myComm;
}

//=============================
// getPersistentComm
//=============================
I_CommPersistent* DCollectiveOp::getPersistentComm (void)
{
    return myComm;
}

//=============================
// getCollId
//=============================
MustCollCommType DCollectiveOp::getCollId (void)
{
    return myCollId;
}

//=============================
// getIssuerRank
//=============================
int DCollectiveOp::getIssuerRank (void)
{
    return myRank;
}

//=============================
// getCommSize
//=============================
int DCollectiveOp::getCommSize (void)
{
    return myCommSize;
}

//=============================
// getRoot
//=============================
int DCollectiveOp::getRoot (void)
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
bool DCollectiveOp::isToOne (void)
{
    return myIsToOne;
}

//=============================
// hasRoot
//=============================
bool DCollectiveOp::hasRoot (void)
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
bool DCollectiveOp::isSendTransfer(void)
{
    return myIsSendTransfer;
}

//=============================
// isReceiveTransfer
//=============================
bool DCollectiveOp::isReceiveTransfer(void)
{
    return myIsReceiveTransfer;
}

//=============================
// isNoTransfer
//=============================
bool DCollectiveOp::isNoTransfer(void)
{
    return (!myIsSendTransfer && !myIsReceiveTransfer);
}

//=============================
// hasOp
//=============================
bool DCollectiveOp::hasOp (void)
{
    return (myOp != NULL);
}

//=============================
// getOp
//=============================
I_Op* DCollectiveOp::getOp (void)
{
    return myOp;
}

//=============================
// getPId
//=============================
MustParallelId DCollectiveOp::getPId (void)
{
    return myPId;
}

//=============================
// getLId
//=============================
MustParallelId DCollectiveOp::getLId (void)
{
    return myLId;
}

//=============================
// requiresSecondOp
//=============================
bool DCollectiveOp::requiresSecondOp (void)
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
// isFirstOpOfWave
//=============================
bool DCollectiveOp::isFirstOpOfWave (void)
{
    //Sends are always first, so only for receives we need to think
    if (myIsReceiveTransfer)
    {
        //Following ops may not have a preceding send op, and thus be first
        if ( (myCollId == MUST_COLL_SCATTER || myCollId == MUST_COLL_SCATTERV) &&
                myRank != myDestSource)
            return true;

        if (myCollId == MUST_COLL_BCAST)
            return true;

        return false;
    }

    return true;
}


//=============================
// printCollectiveMismatch
//=============================
bool DCollectiveOp::printCollectiveMismatch (DCollectiveOp *other)
{
    //Sanity check
    if (myCollId == other->myCollId)
        return true;

    //Ignore if already checked by ancestor in TBON
    if (    myFromChannel >= 0 &&
            other->myFromChannel >= 0 &&
            myFromChannel == other->myFromChannel)
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

    myMatcher->getLogger()->createMessage (
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
bool DCollectiveOp::printRootMismatch (DCollectiveOp *other)
{
    //Sanity check
    if (!hasRoot() || !other->hasRoot() || getRoot() == other->getRoot())
        return true;

    //Ignore if already checked by ancestor in TBON
    if (    myFromChannel >= 0 &&
            other->myFromChannel >= 0 &&
            myFromChannel == other->myFromChannel)
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

    myMatcher->getLogger()->createMessage (
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
bool DCollectiveOp::printOpMismatch (DCollectiveOp *other)
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

    //Ignore if already checked by ancestor in TBON
    if (    myFromChannel >= 0 &&
            other->myFromChannel >= 0 &&
            myFromChannel == other->myFromChannel)
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

    myMatcher->getLogger()->createMessage (
            MUST_ERROR_COLLECTIVE_OP_MISSMATCH,
            myPId,
            myLId,
            MustErrorMessage,
            stream.str(),
            references);

    return true;
}

//=============================
// printBlockingNonBlockingMismatch
//=============================
bool DCollectiveOp::printBlockingNonBlockingMismatch (DCollectiveOp *other)
{
    //Sanity check (Either one must have a request)
    if (    !hasRequest() &&
            !other->hasRequest()
        )
        return true;
    assert (!hasRequest() || !other->hasRequest());

    DCollectiveOp *wRequest = this,
                  *woRequest = other;
    if (!hasRequest())
    {
        wRequest = other;
        woRequest = this;
    }

    //Ignore if already checked by ancestor in TBON
    if (    myFromChannel >= 0 &&
            other->myFromChannel >= 0 &&
            myFromChannel == other->myFromChannel)
        return true;

    std::list <std::pair <MustParallelId, MustLocationId> > references;
    std::stringstream stream;

    stream
        << "The application matches a blocking collective (call location of this message) with a non-blocking collective (call location in reference 1)! "
        << "A correct MPI application must only match blocking or non-blocking collectives with each other, but not mix them. ";

    references.push_back(std::make_pair (wRequest->myPId, wRequest->myLId));

    stream
        << "(Information on MPI communicator: ";
    myComm->printInfo(stream, &references);
    stream << ")";
    stream << std::endl << "Note that collective matching was disabled as a result, collectives won't be analyzed for their correctness or blocking state anymore. You should solve this issue and rerun your application with MUST.";

    myMatcher->getLogger()->createMessage (
            MUST_ERROR_COLLECTIVE_BLOCKING_NONBLOCKING_MISSMATCH,
            woRequest->myPId,
            woRequest->myLId,
            MustErrorMessage,
            stream.str(),
            references);

    return true;
}

//=============================
// validateTypeMatch
//=============================
bool DCollectiveOp::validateTypeMatch (DCollectiveOp* other)
{
    int                 sendCount, receiveCount;
    I_Datatype     *sendType, *receiveType;
    DCollectiveOp  *send, *receive;

    //Ignore if already checked by ancestor in TBON
    if (    myFromChannel >= 0 &&
            other->myFromChannel >= 0 &&
            myFromChannel == other->myFromChannel)
        return true;

    //==1) Sort into send Receive
    /*
     * Note we may also have two sends or two receives, the below is able to handle this!
     */
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
    return matchTypes (
                    send->myPId,
                    send->myLId,
                    send->isSendTransfer(),
                    sendCount,
                    sendType,
                    receive->myPId,
                    receive->myLId,
                    receive->isReceiveTransfer(),
                    receiveCount,
                    receiveType);
}

//=============================
// validateTypeMatch
//=============================
bool DCollectiveOp::validateTypeMatch (MustParallelId pId, MustLocationId lId, I_Datatype* type, int count)
{
    int                 ownCount;
    I_Datatype     *ownType;

    //==2) Determine send/recv count and type
    int otherCommRank;

    if (myCounts || myTypes)
    {
        if (!myComm->getGroup()->containsWorldRank(myMatcher->pIdToRank(pId), &otherCommRank))
        {
            assert (0);//The issuer rank of the receive must be in the comm
            return false;
        }
    }

    if (myCounts)
        ownCount = myCounts[otherCommRank];
    else
        ownCount = myCount;

    if (myTypes)
        ownType = myTypes[otherCommRank];
    else
        ownType = myType;

    //==3) Match the types
    if (isSendTransfer())
        return matchTypes (myPId, myLId, true, ownCount, ownType, pId, lId, true, count, type);

    return matchTypes (pId, lId, true, count, type, myPId, myLId, true, ownCount, ownType);
}

//=============================
// matchTypes
//=============================
bool DCollectiveOp::matchTypes (
                MustParallelId sendPId,
                MustLocationId sendLId,
                bool sendIsSend,
                int sendCount,
                I_Datatype* sendType,
                MustParallelId receivePId,
                MustLocationId receiveLId,
                bool receiveIsReceive,
                int receiveCount,
                I_Datatype* receiveType)
{
    MustMessageIdNames ret = MUST_ERROR_TYPEMATCH_INTERNAL_NOTYPE;
    MustAddressType pos = 0;
    ret = sendType->isEqualB(sendCount, receiveType, receiveCount, &pos);

    std::stringstream stream;
    std::list <std::pair <MustParallelId, MustLocationId> > references;

    if (ret != MUST_ERROR_TYPEMATCH_LENGTH && ret != MUST_ERROR_TYPEMATCH_MISSMATCH)
        return true;

    std::string sendOpName = "the send";
    std::string receiveOpName = "the receive";

    if (!sendIsSend || !receiveIsReceive)
    {
        sendOpName = "this operations";
        receiveOpName = "the other operations";
    }

    switch (ret)
    {
    case MUST_ERROR_TYPEMATCH_MISSMATCH:
    {
        stream
            << "Two collective calls cause a type mismatch!";

        if (sendIsSend && receiveIsReceive)
        {
            stream
            << " This call sends data to the call in reference 1.";
        }
        else
        {
            stream
            << " This collectives (";

            if (sendIsSend)
                stream << "sending";
            else
                stream << "receiving";

            stream
                << " part) type signature must match the signature of the collective in reference 1 (";

            if (receiveIsReceive)
                stream << "receiving";
            else
                stream << "sending";

            stream
                << " part)";
        }

        references.push_back(std::make_pair(receivePId, receiveLId));

        stream
            << " The mismatch occurs at ";

        sendType->printDatatypeLongPos(stream, pos);

        stream << " in " << sendOpName << " type and at ";

        receiveType->printDatatypeLongPos(stream, pos);

        stream << " in " << receiveOpName << " type (consult the MUST manual for a detailed description of datatype positions).";

        break;
    }
    case MUST_ERROR_TYPEMATCH_LENGTH:
    {
        stream
        << "Two collective operations use (datatype,count) pairs that span type signatures of different length!"
        << " Each send and receive transfer of a collective call must use equal type signatures (I.e. same types with potentially different displacements).";

        if (sendIsSend && receiveIsReceive)
        {
            stream
                << " This is the sending operation and the receiving operation is issued at reference 1.";
        }
        else
        {
            stream
                << " This collective operation (";

            if (sendIsSend)
                stream << "sending";
            else
                stream << "receiving";

            stream
            << " part) has an incompatible type signature length with the collective operation in reference 1 (";

            if (receiveIsReceive)
                stream << "receiving";
            else
                stream << "sending";

            stream
                << " part)";
        }

        references.push_back(std::make_pair(receivePId, receiveLId));

        if (sendType->getSize() * sendCount < receiveType->getSize() * receiveCount) //TODO is this really the right thing to do?
        {
            stream
                << " The first element of " << receiveOpName << " type signature that did not fit into " <<sendOpName << " type signature is at ";
            receiveType->printDatatypeLongPos(stream, pos);
            stream << " in " << receiveOpName << " type (consult the MUST manual for a detailed description of datatype positions).";
        }
        else
        {
            stream
                << " The first element of " << sendOpName << " that did not fit into " << receiveOpName << " operation is at ";
            sendType->printDatatypeLongPos(stream, pos);
            stream << " in " << sendOpName << " type (consult the MUST manual for a detailed description of datatype positions).";
        }
        break;
    }
    default:
        return true;
    }

    stream << " (Information on communicator: ";
    myComm->printInfo(stream,&references);
    stream << ")";

    stream
    << " (Information on " << sendOpName << " transfer of count "
    << sendCount
    << " with type:";

    sendType->printInfo(stream, &references);
    stream << ")";

    stream
    << " (Information on " << receiveOpName << " transfer of count "
    << receiveCount
    << " with type:";

    receiveType->printInfo(stream, &references);

    myMatcher->getLogger()->createMessage (
            ret,
            sendPId,
            sendLId,
            MustErrorMessage,
            stream.str(),
            references);

    return false;
}

//=============================
// hasMultipleTypes
//=============================
bool DCollectiveOp::hasMultipleTypes (void)
{
    if (myTypes)
        return true;
    return false;
}

//=============================
// hasMultipleCounts
//=============================
bool DCollectiveOp::hasMultipleCounts (void)
{
    if (myCounts)
        return true;
    return false;
}

//=============================
// collectiveUsesMultipleCounts
//=============================
bool DCollectiveOp::collectiveUsesMultipleCounts (void)
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
// getNumRanks
//=============================
int DCollectiveOp::getNumRanks (void)
{
    return myNumRanks;
}

//=============================
// createReducedRecord
//=============================
void DCollectiveOp::createReducedRecord (int numRanks, int commStride, int commOffset)
{
    //Set the stride representation for the event that we introduce!
    if (commStride != 1)
    {
        if (commStride == 0) commStride = UINT32_MAX;
        (*myMatcher->getSetNextEventStridedFct()) (commOffset, commStride);
    }

    //Create the event
    switch (myCollId)
    {
    case MUST_COLL_GATHER:
        /*
         * Gather has transitive type matching rules,
         * so we stick with the original events
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myDestSource,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
                    );
        }
        else
        {
            (*myMatcher->getRecvBuffersFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_GATHERV:
        /*
         * Gatherv has non-transitiv type matching,
         * if we reduce it we must have had intra-layer
         * comm and finished all type matching already.
         * We still forward reprsentatives of the original
         * events that still contain superfluous type
         * signature information.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myDestSource,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvCountsFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    NULL, //displs
                    myCounts,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_REDUCE:
        /*
         * Reduce has transitive type matching rules,
         * so we stick with the original events
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getOpSendFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myDestSource,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getOpRecvNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_BCAST:
        /*
         * Bcast has transitive type matching rules,
         * so we stick with the original events
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myDestSource,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_SCATTER:
        /*
         * Scatter has transitive type matching rules,
         * so we stick with the original events
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendBuffersFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myDestSource,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_SCATTERV:
        /*
         * Scatterv has non-transitiv type matching,
         * if we reduce it we must have had intra-layer
         * comm and finished all type matching already.
         * We still forward reprsentatives of the original
         * events that still contain superfluous type
         * signature information.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendCountsFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    NULL,
                    myCounts,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myDestSource,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_ALLGATHER:
        /*
         * Allgather has transitive type matching rules,
         * so we stick with the original events
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvBuffersFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_ALLGATHERV:
        /*
         * Allgatherv has transitive type matching rules,
         * we achieve this with a bit of a trick.
         * The specifications of the recvcounts by all partner processes 
         * is redundant, while their counts may differ (given that types also
         * differ to match up signatures), they all specify the same signatures.
         * We compare recvcounts of all the recveive parts a node receives against each 
         * other to create a transivity. (see validateCountsArrayEquality)
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvCountsFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    NULL, //displs
                    myCounts,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_ALLTOALL:
        /*
         * Alltoall all send and recv type signatures are
         * equal on all tasks.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendBuffersFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvBuffersFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_ALLTOALLV:
        /*
         * Intra layer comm takes care of type matching,
         * we still pass type match information for the
         * chosen representatives.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendCountsFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    NULL, //displs
                    myCounts,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvCountsFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    NULL, //displs
                    myCounts,
                    myTypeHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_ALLTOALLW:
        /*
         * Intra layer comm takes care of type matching,
         * we still pass type match information for the
         * chosen representatives.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getSendTypesFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    NULL, //displs
                    myCounts,
                    myTypeHandles,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getRecvTypesFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    NULL, //displs
                    myCounts,
                    myTypeHandles,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_ALLREDUCE:
        /*
         * Allreduce has transitive type matching rules,
         * so we stick with the original events
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getOpSendNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getOpRecvNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_REDUCE_SCATTER:
        /*
         * Reduce_scatter all Recvcounts are the same.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getOpSendCountsFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCounts,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getOpRecvNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_REDUCE_SCATTER_BLOCK:
        /*
         * Reduce_scatter all Recvcounts are the same.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getOpSendBuffersFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getOpRecvNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_SCAN:
        /*
         * Scan transitive.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getOpSendNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getOpRecvNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_EXSCAN:
        /*
         * Exscan transitive.
         */
        if (this->isSendTransfer())
        {
            (*myMatcher->getOpSendNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        else
        {
            (*myMatcher->getOpRecvNFct()) (
                    myPId,
                    myLId,
                    myCollId,
                    0, //buffer
                    myCount,
                    myTypeHandle,
                    myOpHandle,
                    myCommSize,
                    myCommHandle,
                    numRanks,
                    myHasRequest,
                    myRequest
            );
        }
        break;
    case MUST_COLL_BARRIER:
    case MUST_COLL_CART_CREATE:
    case MUST_COLL_CART_SUB:
    case MUST_COLL_COMM_CREATE:
    case MUST_COLL_COMM_DUP:
    case MUST_COLL_COMM_FREE:
    case MUST_COLL_COMM_SPLIT:
    case MUST_COLL_FINALIZE:
    case MUST_COLL_GRAPH_CREATE:
    case MUST_COLL_INTERCOMM_CREATE:
    case MUST_COLL_INTERCOMM_MERGE:
        (*myMatcher->getNoTransferFct()) (
                myPId,
                myLId,
                myCollId,
                myCommHandle,
                numRanks,
                myHasRequest,
                myRequest);
        break;
    default: assert (0);
    }
}

//=============================
// needsIntraCommToCheck
//=============================
bool DCollectiveOp::needsIntraCommToCheck (void)
{
    if (    myCollId == MUST_COLL_ALLTOALLW ||
            myCollId == MUST_COLL_ALLTOALLV ||
            myCollId == MUST_COLL_GATHERV ||
            myCollId == MUST_COLL_SCATTERV)
        return true;
    return false;
}

//=============================
// validateCountsArrayEquality
//=============================
bool DCollectiveOp::validateCountsArrayEquality (DCollectiveOp *other)
{
    //Do both ops have counts?
    if (!myCounts || !other->myCounts)
        return true;

    //Ignore if already checked by ancestor in TBON
    if (    myFromChannel >= 0 &&
            other->myFromChannel >= 0 &&
            myFromChannel == other->myFromChannel)
        return true;

    //Only applies to this operation:
    assert (myCollId == MUST_COLL_ALLGATHERV);

    //check
    for (int i = 0; i < myCommSize; i++)
    {
        MustMessageIdNames ret = MUST_ERROR_TYPEMATCH_INTERNAL_NOTYPE;
        MustAddressType pos = 0;
        ret = myType->isEqualB(myCounts[i], other->myType, other->myCounts[i], &pos);
        std::string opType = "send";
        if (isReceiveTransfer())
            opType = "receive";

        std::stringstream stream;
        std::list <std::pair <MustParallelId, MustLocationId> > references;

        switch (ret)
        {
        case MUST_ERROR_TYPEMATCH_MISSMATCH:
            stream
            << "Two collective calls cause a type mismatch!"
            << " The typesignature associated with " << opType <<"count[" << i << "]=" << myCounts[i] << " and the " << opType <<"type must match the type signature"
            << "associated with " << opType <<"count[" << i << "]=" << other->myCounts[i] << " and the " << opType <<"type of the collective call in reference 1";
            references.push_back(std::make_pair(other->myPId, other->myLId));

            stream
            << " The mismatch occurs at ";

            myType->printDatatypeLongPos(stream, pos);

            stream << " in this collectives type and at ";

            other->myType->printDatatypeLongPos(stream, pos);

            stream << " in other collectives type (consult the MUST manual for a detailed description of datatype positions).";

            break;
        case MUST_ERROR_TYPEMATCH_MISSMATCH_BYTE:
            //TODO
            assert (0);
            break;
        case MUST_ERROR_TYPEMATCH_INTERNAL_NOTYPE:
        case MUST_ERROR_TYPEMATCH_INTERNAL_TYPESIG:
            continue;
        case MUST_ERROR_TYPEMATCH_LENGTH:
            stream
            << "Two collective calls use (datatype,count) pairs that span type signatures of different length!"
            << " The typesignature associated with " << opType <<"count[" << i << "]=" << myCounts[i] << " and the " << opType <<"type must match the type signature"
            << "associated with " << opType <<"count[" << i << "]=" << other->myCounts[i] << " and the " << opType <<"type of the collective call in reference 1";
            references.push_back(std::make_pair(other->myPId, other->myLId));

            if (myType->getSize() * myCounts[i] < other->myType->getSize() * other->myCounts[i]) //TODO is this really the right thing to do?
            {
                stream
                << " The first element of this operations type signature that did not fit into the other type signature is at ";
                myType->printDatatypeLongPos(stream, pos);
                stream << " (consult the MUST manual for a detailed description of datatype positions).";
            }
            else
            {
                stream
                << " The first element of the other operations type signature that did not fit into this operations signature is at ";
                other->myType->printDatatypeLongPos(stream, pos);
                stream << " (consult the MUST manual for a detailed description of datatype positions).";
            }
            break;
        default:
            continue;
        }

        stream << " (Information on communicator: ";
        myComm->printInfo(stream,&references);
        stream << ")";

        stream
        << " (This operations datatype: ";

        myType->printInfo(stream, &references);
        stream << ")";

        stream
        << " (Information on the other operations datatype: ";

        other->myType->printInfo(stream, &references);

        myMatcher->getLogger()->createMessage (
                ret,
                myPId,
                myLId,
                MustErrorMessage,
                stream.str(),
                references);
        return false;

    }

    return true;
}

//=============================
// validateJustCountsArrayEquality
//=============================
bool DCollectiveOp::validateJustCountsArrayEquality (DCollectiveOp *other)
{
    //Do both ops have counts?
    if (!myCounts || !other->myCounts)
        return true;

    //Ignore if already checked by ancestor in TBON
    if (    myFromChannel >= 0 &&
            other->myFromChannel >= 0 &&
            myFromChannel == other->myFromChannel)
        return true;

    //Only applies to this operation:
    assert (myCollId == MUST_COLL_REDUCE_SCATTER);

    //check
    for (int i = 0; i < myCommSize; i++)
    {
        if (myCounts[i] != other->myCounts[i])
        {
            std::stringstream stream;
            std::list <std::pair <MustParallelId, MustLocationId> > references;

            stream
                << "Two collective calls use count arrays that are not equal, while they are required to be equal!"
                << " This operations count[" << i << "]=" << myCounts[i] << " != " << other->myCounts[i]
                << " which is specified for the collective in reference 1";
            references.push_back(std::make_pair(other->myPId, other->myLId));

            stream << " (Information on communicator: ";
            myComm->printInfo(stream,&references);
            stream << ")";

            myMatcher->getLogger()->createMessage (
                    MUST_ERROR_COUNTS_ARRAYS_DIFFER,
                    myPId,
                    myLId,
                    MustErrorMessage,
                    stream.str(),
                    references);
            return false;
        }//are equal?
    }//For counts

    return true;
}

//=============================
// intraCommunicateTypeMatchInfos
//=============================
void DCollectiveOp::intraCommunicateTypeMatchInfos (int waveNumber)
{
    //== Is it any op that we care about?
    if (!needsIntraCommToCheck())
        return;

    //== Even if it is a wave of the right type, is it the right op?
    if ( (myCollId == MUST_COLL_GATHERV && !myIsReceiveTransfer) ||
         (myCollId == MUST_COLL_GATHERV && getRoot() != myRank)   )
        return;

    if ( (myCollId == MUST_COLL_SCATTERV && !myIsSendTransfer) ||
         (myCollId == MUST_COLL_SCATTERV && getRoot() != myRank)   )
        return;

    if (    (myCollId == MUST_COLL_ALLTOALLV && myIsReceiveTransfer) ||
            (myCollId == MUST_COLL_ALLTOALLW && myIsReceiveTransfer))
        return;

    /*==
     * Now the following ops remain:
     * - Gatherv: receiving root op
     * - Scatterv: sending root op
     * - Alltoallv/w: any sending op
     */

    //== Data for distribution
    MustRemoteIdType commRId, typeRId;
    int *counts = myMatcher->getWorldSizedCountArray();
    int maxCountsPerRank;
    MustRemoteIdType* types = NULL;

    //== Get own place id
    int myPlaceId;
    myPlaceId = myMatcher->getLevelIdForApplicationRank (myRank);

    //== How many ranks per place (maxium, assuming current distribution schemes block and uniform) @todo needs to be aware of future distribution schemes
    int curRank =0;
    for (; myMatcher->getLevelIdForApplicationRank (curRank) == 0; curRank++);
    maxCountsPerRank = curRank;

    if (myTypes)
        types = new MustRemoteIdType [myMatcher->getWorldSize()];

    //== For each rank in the comm we now determine the target place and distribute the data
    int lastPlace = -1; //Start with an invalid place id as last place
    int firstRank = -1;
    bool hasData = false;
    int numWorldRanksForPlace = 0;

    for (int i = 0; i <= myMatcher->getWorldSize(); i++)
    {
        int targetPlace;
        MustRemoteIdType tempTypeRId;
        int worldToThisRank;
        bool isInComm;

        /** @todo think about inter communicators here! (MPI-2).*/
        if (i < myMatcher->getWorldSize())
        {
            isInComm = myComm->getGroup()->containsWorldRank(i, &worldToThisRank);
            targetPlace = myMatcher->getLevelIdForApplicationRank (i);
        }
        else
        {
            targetPlace = -1;
        }

        if (targetPlace == myPlaceId)
            continue; //nothing to do, this rank is reachable on this very node anyways

        //== Is this the first thing we send to this place?
        if (lastPlace != targetPlace)
        {
            //Send last !
            if (lastPlace != -1 && hasData)
            {
                if (!myTypes)
                {
                    (*myMatcher->getPassTypeMatchInfoFct()) (
                            myPId,
                            myLId,
                            commRId,
                            typeRId,
                            numWorldRanksForPlace,
                            &(counts[firstRank]),
                            firstRank,
                            waveNumber,
                            myCollId,
                            lastPlace
                    );
                }
                else
                {
                    (*myMatcher->getPassTypeMatchInfoTypesFct()) (
                            myPId,
                            myLId,
                            commRId,
                            numWorldRanksForPlace,
                            &(types[firstRank]),
                            &(counts[firstRank]),
                            firstRank,
                            waveNumber,
                            myCollId,
                            lastPlace
                    );
                }//One type or multiple ones?
            }//Valid target place?

            //If this is the extra iteration we use to send the last counts, break the loop
            if (targetPlace == -1)
                break;

            //Prepare
            firstRank = i;
            hasData = false;
            numWorldRanksForPlace = 0;

            //Send resources!
            myMatcher->getLocationModule()->passLocationToPlace (myPId, myLId, targetPlace);
            myMatcher->getCommTrack()->passCommAcross(myRank, myComm, targetPlace, &commRId);

            //If single type we send the type now
            if (!myTypes)
            {
                myMatcher->getDatatypeTrack()->passDatatypeAcross(myRank, myType, targetPlace, &typeRId);
            }
        }

        //Fill in data
        if (isInComm)
        {
            hasData = true;

            //== Send extra resources
            if (myTypes)
            {
                myMatcher->getDatatypeTrack()->passDatatypeAcross(myRank, myTypes[worldToThisRank], targetPlace, &tempTypeRId);
                types[i] = tempTypeRId;
            }

            //== Add to list of counts
            counts[i] = myCounts[worldToThisRank];
        }
        else
        {
            counts[i] = -1;

            if (myTypes)
            {
                myTypes[i] = 0;
            }
        }

        //Store last place id
        lastPlace = targetPlace;
        numWorldRanksForPlace++;
    }

    if (types) delete [] types;
}

//=============================
// getLTimeStamp
//=============================
MustLTimeStamp DCollectiveOp::getLTimeStamp (void)
{
    return myTS;
}

//=============================
// setLTimeStamp
//=============================
void DCollectiveOp::setLTimeStamp (MustLTimeStamp ts)
{
    myTS = ts;
}

//=============================
// hasRequest
//=============================
bool DCollectiveOp::hasRequest (void)
{
    return myHasRequest;
}

//=============================
// getRequest
//=============================
MustRequestType DCollectiveOp::getRequest (void)
{
    assert (myHasRequest); //Better say that this is not a good query (The doxygen info for the call mentions this)
    return myRequest;
}

/*EOF*/
