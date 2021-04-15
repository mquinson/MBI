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
 * @file P2POp.cpp
 *       @see must::P2POp.
 *
 *  @date 26.07.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "P2POp.h"
#include "sstream"
#include "fstream"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "MustDefines.h"

using namespace must;

//=============================
// Constructor
//=============================
P2POp::P2POp (P2PMatch* matcher, bool isSend, int tag, int toRank, I_CommPersistent* comm, I_DatatypePersistent* datatype, int count, MustParallelId pId, MustLocationId lId, MustSendMode mode)
 :  myMatcher (matcher),
    myIsSend (isSend),
    myTag (tag),
    myToRank (toRank),
    wasWcReceive (false),
    myWcRecvFirstMatchingWorldRank (-1),
    myHasRequest (false),
    myRequest (0),
    myComm (comm),
    myType (datatype),
    myCount (count),
    myPId (pId),
    myLId (lId),
    myIsInSusendedWcOpQueue (false),
    mySendMode (mode)
{
    myRank = myMatcher->myPIdMod->getInfoForId(myPId).rank;

    if (!isSend && toRank == myMatcher->myConsts->getAnySource())
        wasWcReceive = true;
}

//=============================
// Constructor
//=============================
P2POp::P2POp (P2PMatch* matcher, bool isSend, int tag, int toRank, MustRequestType request, I_CommPersistent* comm, I_DatatypePersistent* datatype, int count, MustParallelId pId, MustLocationId lId, MustSendMode mode)
:  myMatcher (matcher),
   myIsSend (isSend),
   myTag (tag),
   myToRank (toRank),
   wasWcReceive (false),
   myWcRecvFirstMatchingWorldRank (-1),
   myHasRequest (true),
   myRequest (request),
   myComm (comm),
   myType (datatype),
   myCount (count),
   myPId (pId),
   myLId (lId),
   myIsInSusendedWcOpQueue (false),
   mySendMode (mode)
{
    myRank = myMatcher->myPIdMod->getInfoForId(myPId).rank;

    if (!isSend && toRank == myMatcher->myConsts->getAnySource())
        wasWcReceive = true;
}

//=============================
// ~P2POp
//=============================
P2POp::~P2POp (void)
{
    if (myComm)
        myComm->erase ();
    myComm = NULL;

    if (myType)
        myType->erase();
    myType = NULL;

    myMatcher = NULL;
}

//=============================
// process
//=============================
PROCESSING_RETURN P2POp::process (int rank)
{
    bool needsSuspension = false;
    bool deleteMyself = false;

    //==Process
    if (myIsSend)
    {
        //== SEND
        if (!myMatcher->findMatchingRecv (this, &needsSuspension))
        {
            if (!needsSuspension)
            {
                myMatcher->addOutstandingSend (this);
            }
            else
            {
                myMatcher->myOrder->suspend();
#ifdef MUST_MATCH_DEBUG
                std::cout << "SUSPENDED (send)" << std::endl;
#endif
                return PROCESSING_REEXECUTE;
            }
        }
        else
        {
            //We where matched, perfect we can kill ourselfes
            deleteMyself =true;
        }
    }
    else
    {
        //Are we still in the queue for suspended wc receives, if so remove ?
        /*
         * The suspendedWcRecvs list for this rank may be empty as the wc source
         * patching task already removes the entry in the list.
         */
        if (myIsInSusendedWcOpQueue && !myMatcher->suspendedWcRecvs[rank].empty())
        {
            if (myMatcher->suspendedWcRecvs[rank].front() == this)
            {
                myMatcher->suspendedWcRecvs[rank].pop_front();
                myIsInSusendedWcOpQueue = false;
            }
        }

        //== RECEIVE
        if (!myMatcher->findMatchingSend (this, &needsSuspension))
        {
            if (!needsSuspension)
            {
                myMatcher->addOutstandingRecv (this);
            }
            else
            {
                myMatcher->myOrder->suspend();
                myMatcher->mySuspendedForEntry = this;
                addToSuspendedWCOpQueue ();

#ifdef MUST_MATCH_DEBUG
std::cout << "SUSPENDED (recv)" << std::endl;
#endif
                return PROCESSING_REEXECUTE;
            }
        }
        else
        {
            deleteMyself =true;
        }
    }

#ifdef MUST_MATCH_DEBUG
    std::cout << "PROCESSED rank="<< myRank <<": ";
    print (std::cout);
    std::cout  << std::endl;
    myMatcher->printQs ();
#endif

    if (deleteMyself)
        delete (this);

    return PROCESSING_SUCCESS;
}

//=============================
// print
//=============================
GTI_RETURN P2POp::print (std::ostream &out)
{
    if (myIsSend)
        out << "Send";
    else
        out << "Recv";

    if (myHasRequest)
        out << " (request=" << myRequest << ")";

    out << " target=" << myToRank << " tag=";

    if (myTag == myMatcher->myConsts->getAnyTag())
        out << "MPI_ANY_TAG";
    else
        out << myTag;

    out << " commSize=" << myComm->getGroup()->getSize() << " typeExtent=" << myType->getExtent() << " count=" << myCount;

    return GTI_SUCCESS;
}

//=============================
// addToSuspendedWCOpQueue
//=============================
void P2POp::addToSuspendedWCOpQueue (void)
{
    if (
            !myIsSend &&
            myToRank == myMatcher->myConsts->getAnySource() &&
            !myIsInSusendedWcOpQueue)
    {
        std::map<int, std::list<P2POp*> >::iterator pos = myMatcher->suspendedWcRecvs.find(myRank);

        if (pos == myMatcher->suspendedWcRecvs.end())
        {
            std::list<P2POp*> temp;
            temp.push_back(this);
            myMatcher->suspendedWcRecvs.insert(std::make_pair(myRank, temp));
        }
        else
        {
            pos->second.push_back (this);
        }
        myIsInSusendedWcOpQueue = true;
    }
}

//=============================
// logAsLost
//=============================
void P2POp::logAsLost (int rank)
{
    std::list <std::pair <MustParallelId, MustLocationId> > references;
    std::stringstream stream;

    std::string type= "send";
    std::string dest = "to";
    if (!myIsSend)
    {
        type = "receive";
        dest = "from";
    }

    stream
        << "The application fails to match a point-to-point operation before it issues MPI_Finalize. "
        << "The outstanding " << type << " point-to-point message of rank " << rank << " needs to be " 
        << "matched by a message " << dest << " rank ";

    if (myToRank == myMatcher->myConsts->getAnySource())
        stream << "MPI_ANY_SOURCE";
    else
        stream << myToRank;

    stream
        << " (both as ranks in MPI_COMM_WORLD). The outstanding " << type << " was activated in the "
        << "source location in reference 1. The operation uses the tag ";

    if (myTag == myMatcher->myConsts->getAnyTag())
        stream << "MPI_ANY_TAG";
    else
        stream << myTag;

    stream << " and the communicator (";
    myComm->printInfo(stream,&references);
    stream << "). A correct application must match all point-to-point communications before entering MPI_Finalize." << std::endl;

    myMatcher->myLogger->createMessage(MUST_ERROR_MESSAGE_LOST, myPId, myLId, MustErrorMessage, stream.str(), references);
}

//=============================
// getToRank
//=============================
int P2POp::getToRank (void)
{
    return myToRank;
}

//=============================
// getComm
//=============================
I_Comm* P2POp::getComm (void)
{
    return myComm;
}

//=============================
// getCommCopy
//=============================
I_CommPersistent* P2POp::getCommCopy (void)
{
    myComm->copy ();
    return myComm;
}

//=============================
// getPersistentComm
//=============================
I_CommPersistent* P2POp::getPersistentComm (void)
{
    return myComm;
}

//=============================
// getComm
//=============================
int P2POp::getIssuerRank (void)
{
    return myRank;
}

//=============================
// matchTags
//=============================
bool P2POp::matchTags (P2POp* other)
{
    if (myIsSend && !other->myIsSend)
    {
        if (other->myTag == myMatcher->myConsts->getAnyTag())
            return true;

        if (myTag == other->myTag)
            return true;
    }
    else if (!myIsSend && other->myIsSend)
    {
        if (myTag == myMatcher->myConsts->getAnyTag())
            return true;

        if (myTag == other->myTag)
            return true;
    }

    return false;
}

//=============================
// hasRequest
//=============================
bool P2POp::hasRequest (void)
{
    return myHasRequest;
}

//=============================
// getRequest
//=============================
MustRequestType P2POp::getRequest (void)
{
    return myRequest;
}

//=============================
// updateToSource
//=============================
void P2POp::updateToSource (int newToRank)
{
    myToRank = newToRank;
}

//=============================
// matchTypes
//=============================
bool P2POp::matchTypes (P2POp* other)
{
    static std::map<MustParallelId, bool> missmatchMap;
    if (!other || !myType || !other->myType)
        return false;

    MustMessageIdNames ret = MUST_ERROR_TYPEMATCH_INTERNAL_NOTYPE;
    MustAddressType pos = 0;

    if (myIsSend && !other->myIsSend)
    {
        ret = myType->isSubsetOfB(myCount, other->myType, other->myCount, &pos);
    }
    else if (!myIsSend && other->myIsSend)
    {
        ret = other->myType->isSubsetOfB(other->myCount, myType, myCount, &pos);
    }
    else
    {
        return false;
    }

    std::stringstream stream;
    switch (ret)
    {
    case MUST_ERROR_TYPEMATCH_MISSMATCH:
        stream
            << "A send and a receive operation use datatypes that do not match!"
            << " Mismatch occurs at ";

        if (myIsSend)
            myType->printDatatypeLongPos(stream, pos);
        else
            other->myType->printDatatypeLongPos(stream, pos);

        stream << " in the send type and at ";

        if (!myIsSend)
            myType->printDatatypeLongPos(stream, pos);
        else
            other->myType->printDatatypeLongPos(stream, pos);

        stream << " in the receive type (consult the MUST manual for a detailed description of datatype positions).";
        if (!missmatchMap[myPId])
        {
            missmatchMap[myPId] = true;
            std::string htmlFile, imageFile, dotFile;
            std::ofstream out;
            std::string callA, callB;
            {
                std::stringstream ss;
                ss << MUST_OUTPUT_DIR<<"MUST_Typemismatch_" << myPId;
                dotFile = ss.str();
            }
            imageFile=dotFile+".png";
            htmlFile=dotFile+".html";
            dotFile=dotFile+".dot";
            MUST_OUTPUT_DIR_CHECK
            out.open (dotFile.c_str());
            if (myIsSend)
            {
                callA = "send"; callB = "recv";
            }
            else
            {
                callA = "recv"; callB = "send";
            }
            {
                std::stringstream sa, sb;
                sa << myMatcher->myLIdMod->getInfoForId(myPId, myLId).callName << ":" <<callA;
                sb << myMatcher->myLIdMod->getInfoForId(other->myPId, other->myLId).callName << ":" <<callB;
                callA = sa.str();
                callB = sb.str();
            }
            myType->printDatatypeDotTypemismatch(out, pos, callA, other->myType, callB);
            out.close();
#ifdef DOT
            generateTypemismatchHtml(dotFile, htmlFile, imageFile);
            stream
                << " A graphical representation of this situation is available in a"
                << " <a href=\""<< htmlFile <<"\" title=\"detailed type mismatch view\"> detailed type mismatch view ("<< htmlFile << ")</a>.";
#else
            stream
                << " A graphical representation of this situation is available in the file named \""<< dotFile <<"\"."
                << " Use the dot tool of the graphviz package to visualize it, e.g. issue \"dot -Tps "<< dotFile <<" -o mismatch.ps\"."
                << " The graph shows the nodes of the involved Datatypes that form the root cause of the type mismatch.";
#endif
        }
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
            << "A receive operation uses a (datatype,count) pair that can not hold the data transfered by the send it matches!"
            << " The first element of the send that did not fit into the receive operation is at ";

        if (myIsSend)
            myType->printDatatypeLongPos(stream, pos);
        else
            other->myType->printDatatypeLongPos(stream, pos);

        stream << " in the send type (consult the MUST manual for a detailed description of datatype positions).";
        break;
    default:
        return true;
    }

    std::list <std::pair <MustParallelId, MustLocationId> > references;

    stream << " The send operation was started at reference 1, the receive operation was started at reference 2.";

    if (myIsSend)
    {
        references.push_back(std::make_pair(myPId, myLId));
        references.push_back(std::make_pair(other->myPId, other->myLId));
    }
    else
    {
        references.push_back(std::make_pair(other->myPId, other->myLId));
        references.push_back(std::make_pair(myPId, myLId));
    }

    stream << " (Information on communicator: ";
    myComm->printInfo(stream,&references);
    stream << ")";

    stream << " (Information on send of count ";

    if (myIsSend)
        stream << myCount;
    else
        stream << other->myCount;
    stream << " with type:";

    if (myIsSend)
        myType->printInfo(stream, &references);
    else
        other->myType->printInfo(stream, &references);
    stream << ")";

    stream << " (Information on receive of count ";

    if (!myIsSend)
        stream << myCount;
    else
        stream << other->myCount;
    stream << " with type:";

    if (!myIsSend)
        myType->printInfo(stream, &references);
    else
        other->myType->printInfo(stream, &references);
    stream << ")";

    myMatcher->myLogger->createMessage (
            ret,
            myPId,
            myLId,
            MustErrorMessage,
            stream.str(),
            references);

    return true;
}

//=============================
// getTag
//=============================
int P2POp::getTag (void)
{
    return myTag;
}

//=============================
// getSendMode
//=============================
MustSendMode P2POp::getSendMode (void)
{
    return mySendMode;
}

//=============================
// wasIssuedAsWcReceive
//=============================
bool P2POp::wasIssuedAsWcReceive (void)
{
    return wasWcReceive;
}

//=============================
// isSend
//=============================
bool P2POp::isSend (void)
{
    return myIsSend;
}

//=============================
// getPId
//=============================
MustParallelId P2POp::getPId (void)
{
    return myPId;
}

//=============================
// getLId
//=============================
MustLocationId P2POp::getLId (void)
{
    return myLId;
}

//=============================
// copy
//=============================
P2POp* P2POp::copy (void)
{
    return new P2POp (this);
}

//=============================
// Constructor (copy from)
//=============================
P2POp::P2POp (P2POp* from)
{
    myMatcher = from->myMatcher;
    myIsSend = from->myIsSend;
    myTag = from->myTag;
    myRank = from->myRank;
    myToRank = from->myToRank;
    wasWcReceive = from->wasWcReceive;
    myHasRequest = from->myHasRequest; /**< True if this send/recv has a request.*/
    myRequest = from->myRequest; /**< Request if present, no persistent info needed, requests are at least as long available as a p2p op.*/

    myComm = from->myComm; /**< The communicator of the send/recv, only set if not available otherwise.*/
    if (myComm) myComm->copy();

    myType = from->myType;
    if (myType) myType->copy();

    myCount = from->myCount;
    myPId = from->myPId;
    myLId = from->myLId;
    myIsInSusendedWcOpQueue = from->myIsInSusendedWcOpQueue;
    mySendMode = from->mySendMode;
    myWcRecvFirstMatchingWorldRank = from->myWcRecvFirstMatchingWorldRank;
}

//=============================
// copyQueuedOp
//=============================
I_Operation* P2POp::copyQueuedOp (void)
{
    P2POp* ret = copy();

    //Now the trick!
    //If we copy this queued operation, we may need to add it to myCheckpointSuspendedWcRecvs
    if (myIsInSusendedWcOpQueue)
    {
        myMatcher->myCheckpointSuspendedWcRecvs[myRank].push_back(ret);

        //Also update the suspension reason
        if (this == myMatcher->mySuspendedForEntry)
            myMatcher->myCheckpointSuspendedForEntry = ret;
    }

    return ret;
}

//=============================
// setFirstWorldRankWithValidMatch
//=============================
void P2POp::setFirstWorldRankWithValidMatch (int worldRank)
{
    myWcRecvFirstMatchingWorldRank = worldRank;
}

//=============================
// setFirstWorldRankWithValidMatch
//=============================
int P2POp::getFirstWorldRankWithValidMatch (void)
{
    return myWcRecvFirstMatchingWorldRank;
}

#ifdef DOT
//=============================
// generateTypemismatchHtml
//=============================
void P2POp::generateTypemismatchHtml (std::string dotFile, std::string htmlFile, std::string imageFile)
{

    std::string command = ((std::string) MUST_TIMEOUT_SCRIPT) + ((std::string) " -t 5 ") + ((std::string) DOT) + ((std::string) " -Tpng ") + dotFile + ((std::string)" -o ") + imageFile;
    system ( command.c_str() );

    //Print the two maps as dot
    std::ofstream out;
    out.open (htmlFile.c_str());

    char buf[128];
    struct tm *ptr;
    time_t tm;
    tm = time(NULL);
    ptr = localtime(&tm);
    strftime(buf ,128 , "%c.\n",ptr);

    //print the header
    out
            << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">" << std::endl
            << "<html>" << std::endl
            << "<head>" << std::endl
            << "<title>MUST type mismatch file</title>" << std::endl
            << "<style type=\"text/css\">" << std::endl
            << "td,td,table {border:thin solid black}" << std::endl
            << "td.ee1{ background-color:#FFDDDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ee2{ background-color:#FFEEEE; text-align:center; vertical-align:middle;}" << std::endl
            << "</style>" << std::endl
            << "</head>" << std::endl
            << "<body>" << std::endl
            << "<p> <b>MUST Type Mismatch Details</b>, date: "
            << buf
            << "</p>" << std::endl
            << "<a href=\""<<MUST_OUTPUT_REDIR<<"MUST_Output.html\" title=\"MUST error report\">Back to MUST error report</a><br>" << std::endl
            << "<table border=\"0\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">" << std::endl


            << "<tr>" << std::endl
            << "<td align=\"center\" bgcolor=\"#9999DD\" colspan=\"2\">" << std::endl
            << "<b>Message</b>"<< std::endl
            << "</td>" << std::endl
            << "</tr>" << std::endl
            << "<tr>" << std::endl
            << "<td class=\"ee2\" colspan=\"3\" >" << std::endl
            << "The application issued a set of MPI calls that mismatch in type signatures! " << std::endl
            << "The graph below shows details on this situation. " << std::endl
            << "The first differing item of each involved communication request is highlighted." << std::endl
            << "</td>" << std::endl
            << "</tr>"  << std::endl

            << "<tr>" << std::endl
            << "<td align=\"center\" bgcolor=\"#7777BB\">"
            << "<b>Datatype Graph</b>"
            << "</td>" << std::endl
            << "</tr>" << std::endl
            << "<tr>" << std::endl
            << "<td class=\"ee2\" ><img src=\""<< MUST_OUTPUT_REDIR << imageFile <<"\" alt=\"type mismatch\"></td>" << std::endl
            << "</tr>" << std::endl


            << "</table>" << std::endl
            << "</body>" << std::endl
            << "</html>" << std::endl;
    out.flush();
    out.close();
}
#endif

/*EOF*/
