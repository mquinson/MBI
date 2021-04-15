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
 * @file OverlapChecks.cpp
 *       @see must::OverlapChecks.
 *
 *  @date 27.05.2011
 *  @author Joachim Protze
 */

#include "GtiMacros.h"
#include "OverlapChecks.h"
#include "MustEnums.h"
#include "MustDefines.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sstream>
#include <fstream>

using namespace must;

mGET_INSTANCE_FUNCTION(OverlapChecks)
mFREE_INSTANCE_FUNCTION(OverlapChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(OverlapChecks)


std::string OverlapChecks::graphFileName(
        MustParallelId pId)
{
    static int index=0;
    std::stringstream ss;
    ss << MUST_OUTPUT_DIR << "MUST_Overlap_" << pId << "_" << index;
    index++;
    return ss.str();
}


int OverlapChecks::pId2Rank(
        MustParallelId pId)
{
    return myPIdMod->getInfoForId(pId).rank;
}

//=============================
// Constructor.
//=============================
OverlapChecks::OverlapChecks (const char* instanceName)
    : ModuleBase<OverlapChecks, I_OverlapChecks> (instanceName),
    doDotOutput(true),
    activeBlocks (),
    preparedBlocklists (),
    activeRequestsBlocklists ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBMODULES 6
    if (subModInstances.size() < NUM_SUBMODULES)
    {
            std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
            assert (0);
    }
    if (subModInstances.size() > NUM_SUBMODULES)
    {
            for (std::vector<I_Module*>::size_type i = NUM_SUBMODULES; i < subModInstances.size(); i++)
                destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myLogger = (I_CreateMessage*) subModInstances[1];
    myArgMod = (I_ArgumentAnalysis*) subModInstances[2];
    myDatMod = (I_DatatypeTrack*) subModInstances[3];
    myReqMod = (I_RequestTrack*) subModInstances[4];
    myLIdMod = (I_LocationAnalysis*) subModInstances[5];

    //Initialize module data
    //Nothing to do
}

//=============================
// Destructor.
//=============================
OverlapChecks::~OverlapChecks (void)
{
    if (myPIdMod)
        destroySubModuleInstance ((I_Module*) myPIdMod);
    myPIdMod = NULL;

    if (myLogger)
        destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myArgMod)
        destroySubModuleInstance ((I_Module*) myArgMod);
    myArgMod = NULL;

    if (myDatMod)
        destroySubModuleInstance ((I_Module*) myDatMod);
    myDatMod = NULL;

    if (myReqMod)
        destroySubModuleInstance ((I_Module*) myReqMod);
    myReqMod = NULL;


#ifdef MUST_DEBUG
    int myCount=0;
    for (mustPidMap< MustMemIntervalListType >::iterator iter1=activeBlocks.begin(); iter1!=activeBlocks.end(); iter1++)
            myCount+=iter1->second.size();
    std::cout << "Sizeof(activeBlocks) = " << myCount << std::endl;

    myCount=0;
    for (mustPidRequestMap< MustMemIntervalListType >::iterator iter1=preparedBlocklists.begin(); iter1!=preparedBlocklists.end(); iter1++)
        for (std::map< MustRequestType, MustMemIntervalListType >::iterator iter2=iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
            myCount+=iter2->second.size();
    std::cout << "Sizeof(preparedBlocklists) = " << myCount << std::endl;

    myCount=0;
    for (mustPidRequestMap< std::list< MustMemIntervalListType::iterator > >::iterator iter1=activeRequestsBlocklists.begin(); iter1!=activeRequestsBlocklists.end(); iter1++)
        for (std::map< MustRequestType, std::list< MustMemIntervalListType::iterator > >::iterator iter2=iter1->second.begin(); iter2!=iter1->second.end(); iter2++)
            myCount+=iter2->second.size();
    std::cout << "Sizeof(activeRequestsBlocklists) = " << myCount << std::endl;
#endif

}


//=============================
//  Helper function for isTypeOverlapped(N)
//=============================

bool OverlapChecks::checkTypeOverlapped (
        I_Datatype * typeinfo,
        int extent,
        int count)
{
    if (typeinfo==NULL)
    {
        return false;
    }
    BlockInfo& blockInfo = typeinfo->getBlockInfo();
    MustStridedBlocklistType blocklist = blockInfo;
    if (blockInfo.overlapped)
    {
        return true;
    }
    if (count > 1) // if we want to check copys, we must duplicate the blocklist first
    {
        MustStridedBlocklistType tempList;
        MustStridedBlocklistType::iterator iter, insertIter=tempList.begin();
        int i;
        for (iter = blocklist.begin(); iter != blocklist.end(); iter++)
        {
            for (i=0; i<count; i++)
            {
                insertIter = tempList.insert(insertIter, StridedBlock(*iter, extent * i, i));
            }
        }
//         tempList.sort();
        blocklist=tempList;
    }
    MustStridedBlocklistType::iterator iter, nextIter;

    // run over the (sorted!) blocklist and check neighbors for overlaps
    MustAddressType posA, posB;
    
    if (isOverlapped(blocklist, iter, nextIter, posA, posB))
    {
//         std::cout << "selfoverlapping buffer on " << iter->repetition << "(th) repetition at ";
//         typeinfo->printDatatypeLongPos(std::cout, iter->pos + iter->first - nextIter->first);
//         std::cout << " with " << nextIter->repetition  << "(th) repetition at ";
//         typeinfo->printDatatypeLongPos(std::cout, nextIter->pos);
//         std::cout << std::endl;
        return true;
    }
    return false;
}


//=============================
// isOverlappedN
//=============================
bool OverlapChecks::isOverlappedN(
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        int count)
{
    I_Datatype * typeinfo = myDatMod->getDatatype(pId, datatype);
    if (typeinfo==NULL){
        return false;
    }
    std::pair<int,int> cachevalues = typeinfo->getSelfoverlapCache();
    if (count<=cachevalues.first)
    {
        return false;
    }
    if (count>=cachevalues.second)
    {
        return true;
    }
    
    MustAddressType extent = typeinfo->getExtent(),
                    true_extent = typeinfo->getTrueExtent();
    int myCount = 1;
    if (count > 1 && extent < true_extent) // copys can only overlap, if extent < true_extent
    {
        // filling the first copy of the datatype (width = 'true_extent')
        // with 'myCount' copies , so that 'myCount' * 'extent' > 'true_extent'
        // -> the start of the (myCount+1)'th copy is after the end of the first copy

        // calculate myCount
        myCount = true_extent / extent + 1;

        // myCount = min(count, myCount)
        // we dont want to do more than count or myCount
        if (count < myCount)
        {
            myCount = count;
        }
    }
    if (myCount<=cachevalues.first)
    {
        return false;
    }
    return setSelfOverlapCache(typeinfo, myCount, checkTypeOverlapped(typeinfo, extent, myCount));
}

//=============================
// set entry to overlapCache
//=============================

bool OverlapChecks::setSelfOverlapCache(
        I_Datatype * typeinfo,
        int count,
        bool overlaps
        )
{
    if (overlaps)
    {
        typeinfo->setMinOverlap(count);
    }
    else
    {
        typeinfo->setMaxNoOverlap(count);
    }
    return overlaps;
}


//=============================
// isSendRecvOverlapped
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::isSendRecvOverlapped (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        int sendcount,
        MustDatatypeType sendtype,
        MustAddressType recvbuf,
        int recvcount,
        MustDatatypeType recvtype
        )
{
    return isSendRecvOverlappedN(pId, lId, sendbuf, NULL, 0, &sendcount, 1, &sendtype, 1, recvbuf, NULL, 0, &recvcount, 1, &recvtype, 1);
}

//=============================
// isSendRecvOverlappedN
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::isSendRecvOverlappedN (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType sendbuf,
        const int *senddispls,
        int senddisplslen,
        const int *sendcounts,
        int sendcountslen,
        const MustDatatypeType *sendtypes,
        int sendtypeslen,
        MustAddressType recvbuf,
        const int *recvdispls,
        int recvdisplslen,
        const int *recvcounts,
        int recvcountslen,
        const MustDatatypeType *recvtypes,
        int recvtypeslen
        )
{
    if (sendcountslen<1 || sendtypeslen<1 || recvcountslen<1 || recvtypeslen<1 || senddisplslen<0 || recvdisplslen<0 || (senddisplslen>0 && senddispls==NULL) || (recvdisplslen>0 && recvdispls==NULL))
    {
        // something went really wrong, exit test
        std::cout << "Implementation error: Malicious call of OverlapChecks::isSendRecvOverlappedN !" << std::endl;
        return GTI_ANALYSIS_SUCCESS; 
    }
    MustMemIntervalListType iList, tList;
    int displacement=0, count = sendcounts[0];
    MustDatatypeType type = sendtypes[0];
    I_Datatype * typeinfo = myDatMod->getDatatype(pId, type);
    if (typeinfo == NULL)
    {
//        std::cout << "Datatype " << type << "not found in isSendRecvOverlappedN" << std::endl;
        return GTI_ANALYSIS_SUCCESS;
    }
    MustAddressType extent = typeinfo->getExtent();
    // get all blocks, used for communication
    // at first blocks from the send part
    for (int i=0; i<sendcountslen; i++)
    {
        if (senddisplslen>1) // we have displacements
        {
            displacement = senddispls[i];
        }
        if (sendcountslen>1) // we have counts
        {
            count = sendcounts[i];
        }
        if (sendtypeslen>1) // we have types, this means bytewise displacements
        {
            type = sendtypes[i];
            typeinfo = myDatMod->getDatatype(pId, type);
            if (typeinfo == NULL)
            {
                return GTI_ANALYSIS_SUCCESS;
            }
        }
        else // we have one type, so displacements are given in extent-width
        {
            displacement *= extent;
        }
        tList = calcIntervalList(typeinfo, sendbuf + displacement, count, (MustRequestType)0, true);
        iList.insert(tList.begin(),tList.end());
    }

    // till now overlaps were harmless and allowed
    bool isSelfoverlaping = false;
    // from now on an overlap is harmful and results in an error

    type = recvtypes[0];
    displacement = 0;
    count = recvcounts[0];
    typeinfo = myDatMod->getDatatype(pId, type);
    if (typeinfo == NULL)
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    extent = typeinfo->getExtent();
    for (int i=0; i<recvcountslen && !isSelfoverlaping; i++)
    {
        if (recvdisplslen>1) // we have displacements
        {
            displacement = recvdispls[i];
        }
        if (recvcountslen>1) // we have counts
        {
            count = recvcounts[i];
        }
        if (recvtypeslen>1) // we have types, this means bytewise displacements
        {
            type = recvtypes[i];
            typeinfo = myDatMod->getDatatype(pId, type);
            if (typeinfo == NULL)
            {
                return GTI_ANALYSIS_SUCCESS;
            }
        }
        else // we have one type, so displacements are given in extent-width
        {
            displacement *= extent;
        }
        tList = calcIntervalList(typeinfo, recvbuf + displacement, count, (MustRequestType)0, false, &isSelfoverlaping);
        iList.insert(tList.begin(), tList.end());
    }
    if (isSelfoverlaping)
    {
        // selfoverlaping has been seen with other test!
        return GTI_ANALYSIS_SUCCESS;
    }

    MustMemIntervalListType::iterator iter, nextIter;

    // selfoverlaping -> error !
    MustAddressType posA, posB;
    if (isOverlapped(iList, iter, nextIter, posA, posB, true))
    {
        std::stringstream stream;
        // selfoverlap is found before!
/*        if (iter->type == nextIter->type)
            return GTI_ANALYSIS_SUCCESS;*/
        if (iter->isSend)
            std::swap(posA, posB);
        MustMemIntervalListType::iterator recvIter = (!iter->isSend) ? iter : nextIter,
                                          sendIter = (!iter->isSend) ? nextIter : iter;
        stream
            <<  "The memory regions spanned by the recv part overlaps at the "
            <<  recvIter->repetition << "(th) repetition of datatype at its position ";
        recvIter->type->printDatatypeLongPos(stream, posA);
        stream
            << " with regions spanned by the ";
        if (!sendIter->merged)
        {
            stream << sendIter->repetition << "(th) repetition of datatype at its position ";
            sendIter->type->printDatatypeLongPos(stream, posB);
            stream << " used in ";
        }
        if (sendIter->isSend)
        {
            stream << "send";
        }
        else
        {
            stream << "receive";
        }
        stream << " part of this operation!" << std::endl;
//         else
//             return GTI_ANALYSIS_SUCCESS;

        if(doDotOutput)
        { // generate dotfile
            doDotOutput=false;
            std::string htmlFile, imageFile, dotFile=graphFileName(pId);
            htmlFile = dotFile + ".html";
            imageFile = dotFile + ".png";
            dotFile = dotFile + ".dot";
            std::ofstream out;
            MUST_OUTPUT_DIR_CHECK
            out.open(dotFile.c_str());
            std::string callA=myLIdMod->getInfoForId(pId , lId).callName, callB;
            {
                std::stringstream as,bs;
                as << callA << ":" << "recv";
                if (sendIter->isSend)
                    bs << callA << ":" << "send";
                else
                    bs << callA << ":" << "recv";
                callA = as.str();
                callB = bs.str();
            }
            recvIter->type->printDatatypeDotOverlap(out, posA, recvIter->baseAddress, callA, sendIter->type, posB, sendIter->baseAddress, callB);
            out.close();
#ifdef DOT
            generateOverlapHtml(dotFile, htmlFile, imageFile);
            stream
                << " A graphical representation of this situation is available in a"
                << " <a href=\""<< htmlFile <<"\" title=\"detailed overlap view\"> detailed overlap view ("<< htmlFile << ")</a>.";
#else
            stream
                << " A graphical representation of this situation is available in the file named \""<< dotFile <<"\"."
                << " Use the dot tool of the graphviz package to visualize it, e.g. issue \"dot -Tps "<< dotFile <<" -o overlap.ps\"."
                << " The graph shows the nodes of the involved Datatypes that form the root cause of the overlap, nodes of the Datatype, that reference to distinct addresses are removed.";
#endif
        }

        
        myLogger->createMessage(
                MUST_ERROR_SELFOVERLAPPED,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// isTypeOverlapped
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::isTypeOverlapped (
        MustParallelId pId,
        MustLocationId lId,
        int aId,
        MustDatatypeType datatype)
{
    return isTypeOverlappedN(pId, lId, aId, datatype, 1);
}

//=============================
// isTypeOverlappedN
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::isTypeOverlappedN (
        MustParallelId pId,
        MustLocationId lId,
        int aId,
        MustDatatypeType datatype,
        int count)
{
    if (isOverlappedN(pId, lId, datatype, count))
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;
        stream
            <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
            << ") is selfoverlapping after repetition of " << count << " !" << std::endl << "Infos on Datatype:";
            
        myDatMod->getDatatype(pId, datatype)->printInfo(stream, &refs);

        myLogger->createMessage(
                MUST_ERROR_SELFOVERLAPPED,
                pId,
                lId,
                MustErrorMessage,
                stream.str(), 
                refs
                );
        return GTI_ANALYSIS_FAILURE;
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// warnIfTypeOverlapped
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::warnIfTypeOverlapped (
        MustParallelId pId,
        MustLocationId lId,
        int aId,
        MustDatatypeType datatype)
{
    if (isOverlappedN(pId, lId, datatype, 1))
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;
        stream
            <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
            << ") is selfoverlapping !" << std::endl << "Infos on Datatype:";
            
        myDatMod->getDatatype(pId, datatype)->printInfo(stream, &refs);

        myLogger->createMessage(
                MUST_WARNING_SELFOVERLAPPED,
                pId,
                lId,
                MustWarningMessage,
                stream.str(), 
                refs
                );
        return GTI_ANALYSIS_FAILURE;
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// warnIfTypeOverlapped
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::warnIfTypeOverlappedN (
        MustParallelId pId,
        MustLocationId lId,
        int aId,
        MustDatatypeType datatype,
        int count)
{
    if (isOverlappedN(pId, lId, datatype, count))
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;
        stream
            <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
            << ") is selfoverlapping after repetition of " << count << " !" << std::endl << "Infos on Datatype:";
            
        myDatMod->getDatatype(pId, datatype)->printInfo(stream, &refs);

        myLogger->createMessage(
                MUST_WARNING_SELFOVERLAPPED,
                pId,
                lId,
                MustWarningMessage,
                stream.str(), 
                refs
                );
        return GTI_ANALYSIS_FAILURE;
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// isRecvOverlappedN
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::isRecvOverlappedN (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        int count)
{
    if (isOverlappedN(pId, lId, datatype, count))
    {
        std::stringstream stream;
        stream
            <<  "Datatype used for receive is selfoverlapping after repetition of " << count << " !" << std::endl;

        myLogger->createMessage(
                MUST_ERROR_SELFOVERLAPPED,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// isSendOverlappedN
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::isSendOverlappedN (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        int count)
{
    if (isOverlappedN(pId, lId, datatype, count))
    {
        std::stringstream stream;
        stream
            <<  "Datatype used for send is selfoverlapping after repetition of " << count << " !" << std::endl;

        myLogger->createMessage(
                MUST_WARNING_SELFOVERLAPPED,
                pId,
                lId,
                MustWarningMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// sendOverlapcheckCounts
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::sendOverlapcheckCounts (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        MustDatatypeType datatype,
        int commsize,
	int hasRequest,
	MustRequestType request
        )
{
    if(buffer == MUST_IN_PLACE)
        return GTI_ANALYSIS_SUCCESS;
    if(buffer == MUST_BOTTOM)
        buffer = 0;
    MustMemIntervalListType iList, tList;
    bool isSelfoverlaping = false;
    I_Datatype * typeinfo = myDatMod->getDatatype(pId, datatype);
    if (typeinfo == NULL || displs == NULL || counts == NULL)
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    MustAddressType extent = typeinfo->getExtent();
    // get all blocks, used for communication
    if (!hasRequest)
	request=0;
    for (int i=0; i<commsize; i++)
    {
        tList = calcIntervalList(typeinfo, buffer+displs[i]*extent, counts[i], request, true, &isSelfoverlaping);
        iList.insert(tList.begin(), tList.end());
    }

    MustMemIntervalListType::iterator iter, nextIter;

    // selfoverlaping -> warning for send, finish check
    MustAddressType posA, posB;
    
    if (isOverlapped(iList, iter, nextIter, posA, posB, false))
    {
        std::stringstream stream;
        stream
            <<  "data used for send is selfoverlapping with the given combination of displacements and counts!" << std::endl;

        if(doDotOutput)
        { // generate dotfile
            doDotOutput=false;
            std::string htmlFile, imageFile, dotFile=graphFileName(pId);
            htmlFile = dotFile + ".html";
            imageFile = dotFile + ".png";
            dotFile = dotFile + ".dot";
            std::ofstream out;
            MUST_OUTPUT_DIR_CHECK
            out.open(dotFile.c_str());
            std::string callA=myLIdMod->getInfoForId(pId , lId).callName, callB;
            {
                std::stringstream as,bs;
                as << callA << ":" << "send";
                bs << callA << ":" << "send";
                callA = as.str();
                callB = bs.str();
            }
            iter->type->printDatatypeDotOverlap(out, posA, iter->baseAddress, callA, nextIter->type, posB, nextIter->baseAddress, callB);
            out.close();
#ifdef DOT
            generateOverlapHtml(dotFile, htmlFile, imageFile);
            stream
                << " A graphical representation of this situation is available in a"
                << " <a href=\""<< htmlFile <<"\" title=\"detailed overlap view\"> detailed overlap view ("<< htmlFile << ")</a>.";
#else
            stream
                << " A graphical representation of this situation is available in the file named \""<< dotFile <<"\"."
                << " Use the dot tool of the graphviz package to visualize it, e.g. issue \"dot -Tps "<< dotFile <<" -o overlap.ps\"."
                << " The graph shows the nodes of the involved Datatypes that form the root cause of the overlap, nodes of the Datatype, that reference to distinct addresses are removed.";
#endif
        }
        myLogger->createMessage(
                MUST_WARNING_SELFOVERLAPPED,
                pId,
                lId,
                MustWarningMessage,
                stream.str()
                );
    }
    // check for overlaps with pending requests
    GTI_ANALYSIS_RETURN res = checkOverlapsRequests(pId, lId, iList, true, &must::OverlapChecks::outputSendOverlapsRequests);
    if (hasRequest)
	announceRequest(pId, lId, iList, request);
    return res;
}

//=============================
// recvOverlapcheckCounts
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::recvOverlapcheckCounts (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        MustDatatypeType datatype,
        int commsize,
	int hasRequest,
	MustRequestType request
        )
{
    if(buffer == MUST_IN_PLACE)
        return GTI_ANALYSIS_SUCCESS;
    if(buffer == MUST_BOTTOM)
        buffer = 0;
    MustMemIntervalListType iList, tList;
    bool isSelfoverlaping = false;
    I_Datatype * typeinfo = myDatMod->getDatatype(pId, datatype);
    if (typeinfo == NULL || displs == NULL || counts == NULL)
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    MustAddressType extent = typeinfo->getExtent();
    if (!hasRequest)
	request=0;
    // get all blocks, used for communication
    for (int i=0; i<commsize; i++)
    {
        tList = calcIntervalList(typeinfo, buffer+displs[i]*extent, counts[i], request, false, &isSelfoverlaping);
        iList.insert(tList.begin(), tList.end());
    }

    MustMemIntervalListType::iterator iter, nextIter;
    MustAddressType posA, posB;

    // selfoverlaping -> error for recv, quit check
    if (isOverlapped(iList, iter, nextIter, posA, posB, true))
    {
        std::stringstream stream;
        stream
            <<  "data used for receive is selfoverlapping with the given combination of displacements and counts!" << std::endl;
        if(doDotOutput)
        { // generate dotfile
            doDotOutput=false;
            std::string htmlFile, imageFile, dotFile=graphFileName(pId);
            htmlFile = dotFile + ".html";
            imageFile = dotFile + ".png";
            dotFile = dotFile + ".dot";
            std::ofstream out;
            MUST_OUTPUT_DIR_CHECK
            out.open(dotFile.c_str());
            std::string callA=myLIdMod->getInfoForId(pId , lId).callName, callB;
            {
                std::stringstream as,bs;
                as << callA << ":" << "recv";
                bs << callA << ":" << "recv";
                callA = as.str();
                callB = bs.str();
            }
            iter->type->printDatatypeDotOverlap(out, posA, iter->baseAddress, callA, nextIter->type, posB, nextIter->baseAddress, callB);
            out.close();
#ifdef DOT
            generateOverlapHtml(dotFile, htmlFile, imageFile);
            stream
                << " A graphical representation of this situation is available in a"
                << " <a href=\""<< htmlFile <<"\" title=\"detailed overlap view\"> detailed overlap view ("<< htmlFile << ")</a>.";
#else
            stream
                << " A graphical representation of this situation is available in the file named \""<< dotFile <<"\"."
                << " Use the dot tool of the graphviz package to visualize it, e.g. issue \"dot -Tps "<< dotFile <<" -o overlap.ps\"."
                << " The graph shows the nodes of the involved Datatypes that form the root cause of the overlap, nodes of the Datatype, that reference to distinct addresses are removed.";
#endif
        }

        myLogger->createMessage(
                MUST_ERROR_SELFOVERLAPPED,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    // check for overlaps with pending requests
    GTI_ANALYSIS_RETURN res = checkOverlapsRequests(pId, lId, iList, false, &must::OverlapChecks::outputRecvOverlapsRequests);
    if (hasRequest)
	announceRequest(pId, lId, iList, request);
    return res;
}


//=============================
// sendOverlapcheckTypes
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::sendOverlapcheckTypes (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        const MustDatatypeType datatypes[],
        int commsize,
	int hasRequest,
	MustRequestType request
        )
{
    if(buffer == MUST_IN_PLACE)
        return GTI_ANALYSIS_SUCCESS;
    if(buffer == MUST_BOTTOM)
        buffer = 0;
    MustMemIntervalListType iList, tList;
    I_Datatype * typeinfo;

    if(displs == NULL || counts == NULL)
    {
    	return GTI_ANALYSIS_SUCCESS;
    }
    bool isSelfoverlaping = false;
    if (!hasRequest)
	request=0;
    // get all blocks, used for communication
    for (int i=0; i<commsize; i++)
    {
        typeinfo = myDatMod->getDatatype(pId, datatypes[i]);
        if (typeinfo == NULL)
        {
            return GTI_ANALYSIS_SUCCESS;
        }
        tList = calcIntervalList(typeinfo, buffer+displs[i], counts[i], request, true, &isSelfoverlaping);
        iList.insert(tList.begin(),tList.end());
    }

    MustMemIntervalListType::iterator iter, nextIter;

    MustAddressType posA, posB;
    // selfoverlaping -> warning for send, finish check
    if (isOverlapped(iList, iter, nextIter, posA, posB, false))
    {
        std::stringstream stream;
        stream
            <<  "data used for send is selfoverlapping with the given combination of displacements and counts!" << std::endl;

        if(doDotOutput)
        { // generate dotfile
            doDotOutput=false;
            std::string htmlFile, imageFile, dotFile=graphFileName(pId);
            htmlFile = dotFile + ".html";
            imageFile = dotFile + ".png";
            dotFile = dotFile + ".dot";
            std::ofstream out;
            MUST_OUTPUT_DIR_CHECK
            out.open(dotFile.c_str());
            std::string callA=myLIdMod->getInfoForId(pId , lId).callName, callB;
            {
                std::stringstream as,bs;
                as << callA << ":" << "send";
                bs << callA << ":" << "send";
                callA = as.str();
                callB = bs.str();
            }
            iter->type->printDatatypeDotOverlap(out, posA, iter->baseAddress, callA, nextIter->type, posB, nextIter->baseAddress, callB);
            out.close();
#ifdef DOT
            generateOverlapHtml(dotFile, htmlFile, imageFile);
            stream
                << " A graphical representation of this situation is available in a"
                << " <a href=\""<< htmlFile <<"\" title=\"detailed overlap view\"> detailed overlap view ("<< htmlFile << ")</a>.";
#else
            stream
                << " A graphical representation of this situation is available in the file named \""<< dotFile <<"\"."
                << " Use the dot tool of the graphviz package to visualize it, e.g. issue \"dot -Tps "<< dotFile <<" -o overlap.ps\"."
                << " The graph shows the nodes of the involved Datatypes that form the root cause of the overlap, nodes of the Datatype, that reference to distinct addresses are removed.";
#endif
        }
        myLogger->createMessage(
                MUST_WARNING_SELFOVERLAPPED,
                pId,
                lId,
                MustWarningMessage,
                stream.str()
                );
    }
    // check for overlaps with pending requests
    GTI_ANALYSIS_RETURN res = checkOverlapsRequests(pId, lId, iList, true, &must::OverlapChecks::outputSendOverlapsRequests);
    if (hasRequest)
	announceRequest(pId, lId, iList, request);
    return res;
}


//=============================
// recvOverlapcheckTypes
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::recvOverlapcheckTypes (
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType buffer,
        const int displs[],
        const int counts[],
        const MustDatatypeType datatypes[],
        int commsize,
	int hasRequest,
	MustRequestType request
        )
{
    if(buffer == MUST_IN_PLACE)
        return GTI_ANALYSIS_SUCCESS;
    if(buffer == MUST_BOTTOM)
        buffer = 0;
    MustMemIntervalListType iList, tList;
    bool isSelfoverlaping = false;
    I_Datatype * typeinfo;

    if(displs == NULL || counts == NULL || datatypes == NULL)
    {
    	return GTI_ANALYSIS_SUCCESS;
    }
    if (!hasRequest)
	request=0;
    // get all blocks, used for communication
    for (int i=0; i<commsize; i++)
    {
        typeinfo = myDatMod->getDatatype(pId, datatypes[i]);
        if (typeinfo == NULL)
        {
            return GTI_ANALYSIS_SUCCESS;
        }
        tList = calcIntervalList(typeinfo, buffer+displs[i], counts[i], request, false, &isSelfoverlaping);
        iList.insert(tList.begin(),tList.end());
    }

    MustMemIntervalListType::iterator iter, nextIter;

    MustAddressType posA, posB;
    // selfoverlaping -> error for recv, quit check
    if (isOverlapped(iList, iter, nextIter, posA, posB, true))
    {
        std::stringstream stream;
        stream
            <<  "data used for receive is selfoverlapping with the given combination of displacements and counts!" << std::endl;

        if(doDotOutput)
        { // generate dotfile
            doDotOutput=false;
            std::string htmlFile, imageFile, dotFile=graphFileName(pId);
            htmlFile = dotFile + ".html";
            imageFile = dotFile + ".png";
            dotFile = dotFile + ".dot";
            std::ofstream out;
            MUST_OUTPUT_DIR_CHECK
            out.open(dotFile.c_str());
            std::string callA=myLIdMod->getInfoForId(pId , lId).callName, callB;
            {
                std::stringstream as,bs;
                as << callA << ":" << "recv";
                bs << callA << ":" << "recv";
                callA = as.str();
                callB = bs.str();
            }
            iter->type->printDatatypeDotOverlap(out, posA, iter->baseAddress, callA, nextIter->type, posB, nextIter->baseAddress, callB);
            out.close();
#ifdef DOT
            generateOverlapHtml(dotFile, htmlFile, imageFile);
            stream
                << " A graphical representation of this situation is available in a"
                << " <a href=\""<< htmlFile <<"\" title=\"detailed overlap view\"> detailed overlap view ("<< htmlFile << ")</a>.";
#else
            stream
                << " A graphical representation of this situation is available in the file named \""<< dotFile <<"\"."
                << " Use the dot tool of the graphviz package to visualize it, e.g. issue \"dot -Tps "<< dotFile <<" -o overlap.ps\"."
                << " The graph shows the nodes of the involved Datatypes that form the root cause of the overlap, nodes of the Datatype, that reference to distinct addresses are removed.";
#endif
        }
        myLogger->createMessage(
                MUST_ERROR_SELFOVERLAPPED,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    // check for overlaps with pending requests
    GTI_ANALYSIS_RETURN res = checkOverlapsRequests(pId, lId, iList, false, &must::OverlapChecks::outputRecvOverlapsRequests);
    if (hasRequest)
	announceRequest(pId, lId, iList, request);
    return res;
}


//=============================
// calculate Intervallist from blocklist
//=============================

MustMemIntervalListType OverlapChecks::calcIntervalList(
        I_Datatype * typeinfo,
        MustAddressType buffer,
        int count,
        MustRequestType request,
        bool isSend,
        bool* hasoverlap)
{
    static MustMemIntervalListType ret;
    static bool myoverlap=false;
    static I_Datatype * lastinfo=NULL;
    static MustAddressType lastbuffer=0;
    static int lastcount=0;
    static MustRequestType lastrequest=0;
    if(typeinfo == lastinfo && lastcount == count && lastbuffer == buffer && request == lastrequest)
    {
        if (hasoverlap != NULL)
        {
            *hasoverlap = *hasoverlap || myoverlap;
        }
        return ret;
    }
    ret.clear();
    lastinfo = typeinfo;
    lastcount = count;
    lastbuffer = buffer;
    lastrequest = request;
    { // get the blocklist for the datatype and repeate as requested (count)
        BlockInfo& blockInfo = typeinfo->getBlockInfo();
        myoverlap = blockInfo.overlapped;
        MustAddressType extent = typeinfo->getExtent();
        MustAddressType size = typeinfo->getSize();
        ret = buildMemIntervallist(blockInfo, extent, size, buffer, request, isSend, typeinfo, count, buffer);
        if (!myoverlap)
        {
            MustAddressType posA, posB;
            MustMemIntervalListType::iterator iter, nextIter;
            myoverlap = isOverlapped(ret, iter, nextIter, posA, posB, false);
        }
    }
    if (hasoverlap != NULL)
    {
        *hasoverlap = *hasoverlap || myoverlap;
    }
    return ret;
}

//=============================
// mark prepared blocklist as active
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::makeBlocksActive(
        MustParallelId pId,
        MustLocationId lId,
        MustMemIntervalListType & preparedList,
        MustRequestType request
        )
{
    int rank = pId2Rank(pId);
    requestLocation[rank][request] = std::make_pair(pId, lId);
    MustMemIntervalListType::iterator iter;
    std::list< MustMemIntervalListType::iterator > & activeRequestsBlocklist = activeRequestsBlocklists[rank][request];
    MustMemIntervalListType & myActiveBlocks = activeBlocks[rank];
    MustMemIntervalListType::iterator activIter = myActiveBlocks.begin();
    for (iter = preparedList.begin(); iter != preparedList.end(); iter++)
    {
        // insert into activeBlocks and save the iterator for deletion
        activIter = myActiveBlocks.insert(activIter, *iter);
        activeRequestsBlocklist.push_back(activIter);
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// remove blocklist from active list
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::makeBlocksInActive(
        MustParallelId pId,
        MustRequestType request
        )
{
    int rank = pId2Rank(pId);
    return makeBlocksInActive(rank, request);
}

GTI_ANALYSIS_RETURN OverlapChecks::makeBlocksInActive(
        int rank,
        MustRequestType request
        )
{
    // forget the location of a request
    requestLocation[rank].erase(request);
    std::list< MustMemIntervalListType::iterator >::iterator iter;
    for (iter = activeRequestsBlocklists[rank][request].begin(); iter != activeRequestsBlocklists[rank][request].end(); iter++)
    {
        activeBlocks[rank].erase(*iter);
    }

    // after deleting the blocks from activeBlocks, delete the list of bookmarks
    activeRequestsBlocklists[rank].erase(request);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// mark prepared blocklist as active
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::checkOverlapsRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustMemIntervalListType & iList,
        bool isSend,
        void (must::OverlapChecks::*outputFunction)(MustParallelId,MustLocationId,MustRequestType))
{
    int rank = pId2Rank(pId);
    MustAddressType posA, posB;
    MustMemIntervalListType::iterator iterA, iterB;
    if(isOverlapped(activeBlocks[rank], iList, iterA, iterB, posA, posB, true, true))
    {
                (this->*outputFunction)(pId, lId, iterA->request);
                return GTI_ANALYSIS_FAILURE;
    }
    return GTI_ANALYSIS_SUCCESS;
}

GTI_ANALYSIS_RETURN OverlapChecks::checkOverlapsRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustMemIntervalListType & iList,
        bool isSend,
        const char* errstring, enum MustMessageIdNames errcode)
{
    int rank = pId2Rank(pId);
    MustAddressType posA, posB;
    MustMemIntervalListType::iterator iterA, iterB;
    if(isOverlapped(activeBlocks[rank], iList, iterA, iterB, posA, posB, true, true))
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;

        stream
            <<  errstring << std::endl
            << std::endl;

        I_Request* rInfoA = myReqMod->getRequest (pId, iterA->request);
        if (rInfoA && !rInfoA->isNull())
        {
            stream
                << "(Information on the request associated with the other communication:" << std::endl;
                rInfoA->printInfo (stream, &refs);
                stream << ")" << std::endl;
        }

        stream 
            << "(Information on the datatype associated with the other communication:" << std::endl;
        iterA->type->printInfo (stream, &refs);
        stream << ")" << std::endl
            << "The other communication overlaps with this communication at position:";
        iterA->type->printDatatypeLongPos(stream, posA);

        stream
                << std::endl << std::endl;

        I_Request* rInfoB = myReqMod->getRequest (pId, iterB->request);
        if (rInfoB && !rInfoB->isNull())
        {
            stream
                    << "(Information on the request associated with this communication:" << std::endl;
            rInfoB->printInfo (stream, &refs);
            stream << ")" << std::endl;
        }

        stream 
            << "(Information on the datatype associated with this communication:" << std::endl;
        iterB->type->printInfo (stream, &refs);
        stream << ")" << std::endl
            << "This communication overlaps with the other communication at position:";
        iterB->type->printDatatypeLongPos(stream, posB);
        stream  << std::endl;

        
        if(doDotOutput)
        { // generate dotfile
            doDotOutput=false;
            std::string htmlFile, imageFile, dotFile=graphFileName(pId);
            htmlFile = dotFile + ".html";
            imageFile = dotFile + ".png";
            dotFile = dotFile + ".dot";
            std::ofstream out;
            MUST_OUTPUT_DIR_CHECK
            out.open(dotFile.c_str());
            std::string callA=myLIdMod->getInfoForId(pId , lId).callName, callB=callA;
            if (rInfoA && !rInfoA->isNull())
            {
                callA=myLIdMod->getInfoForId(rInfoA->getCreationPId() , rInfoA->getCreationLId()).callName;
            }
            if (rInfoB && !rInfoB->isNull())
            {
                callB=myLIdMod->getInfoForId(rInfoB->getCreationPId() , rInfoB->getCreationLId()).callName;
            }
            {
                std::stringstream as,bs;
                if (iterA->isSend)
                    as << callA << ":" << "send";
                else
                    as << callA << ":" << "recv";
                if (iterB->isSend)
                    bs << callB << ":" << "send";
                else
                    bs << callB << ":" << "recv";
                callA = as.str();
                callB = bs.str();
            }
            iterA->type->printDatatypeDotOverlap(out, posA, iterA->baseAddress, callA, iterB->type, posB, iterB->baseAddress, callB);
            out.close();
#ifdef DOT
            generateOverlapHtml(dotFile, htmlFile, imageFile);
            stream
                << " A graphical representation of this situation is available in a"
                << " <a href=\""<< htmlFile <<"\" title=\"detailed overlap view\"> detailed overlap view ("<< htmlFile << ")</a>.";
#else
            stream
                << " A graphical representation of this situation is available in the file named \""<< dotFile <<"\"."
                << " Use the dot tool of the graphviz package to visualize it, e.g. issue \"dot -Tps "<< dotFile <<" -o overlap.ps\"."
                << " The graph shows the nodes of the involved Datatypes that form the root cause of the overlap, nodes of the Datatype, that reference to distinct addresses are removed.";
#endif
        }

        myLogger->createMessage(
                errcode,
                pId,
                lId,
                MustErrorMessage,
                stream.str(),
                refs
                );
        return GTI_ANALYSIS_FAILURE;
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// prepared error message, given to the check as callback
//=============================
void OverlapChecks::outputSendOverlapsRequests(
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
    std::list<std::pair<MustParallelId,MustLocationId> > refs;
    std::stringstream stream;
    stream
        <<  "The memory regions to be transfered by this send operation overlap with regions spanned by a pending non-blocking receive operation!" << std::endl
        << "(Information on the request associated with the receive:" << std::endl;
    I_Request* rInfo = myReqMod->getRequest (pId, request);
    if (rInfo) 
    {
        rInfo->printInfo (stream, &refs);
    }
    else 
    {
        stream << "unknown request";
    }
    stream << ")" << std::endl;

    myLogger->createMessage(
            MUST_ERROR_OVERLAPPED_SEND,
            pId,
            lId,
            MustErrorMessage,
            stream.str(),
            refs
            );
}

//=============================
// sendOverlapsRequests
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::sendOverlapsRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count)
{
	return isendOverlapsRequests(pId, lId, datatype, buffer, count, 0, (MustRequestType)0 );
}

GTI_ANALYSIS_RETURN OverlapChecks::isendOverlapsRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
	int hasRequest,
	MustRequestType request)
{
    if(buffer == MUST_IN_PLACE)
        return GTI_ANALYSIS_SUCCESS;
    if(buffer == MUST_BOTTOM)
        buffer = 0;
    I_Datatype * typeinfo = myDatMod->getDatatype(pId, datatype);
    if (!hasRequest)
        request=0;
    if (typeinfo == NULL)
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    MustMemIntervalListType iList;
    if (! activeBlocks[pId2Rank(pId)].empty() || hasRequest )
    {
        iList = calcIntervalList(typeinfo, buffer, count, request, true);
    }
    else
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    GTI_ANALYSIS_RETURN res;
    if ( ! activeBlocks[pId2Rank(pId)].empty() )
        res = checkOverlapsRequests(pId, lId, iList, true, "The memory regions to be transfered by this send operation overlap with regions spanned by a pending non-blocking receive operation!", MUST_ERROR_OVERLAPPED_SEND);
    if (hasRequest)
    {
        announceRequest(pId, lId, iList, request);
    }
    return res;
}

//=============================
// prepared error message, given to the check as callback
//=============================

void OverlapChecks::outputRecvOverlapsRequests(
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
    std::list<std::pair<MustParallelId,MustLocationId> > refs;
    std::stringstream stream;
    stream
        <<  "The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!" << std::endl
        << "(Information on the request associated with the other communication:" << std::endl;
    I_Request* rInfo = myReqMod->getRequest (pId, request);
    if (rInfo) rInfo->printInfo (stream, &refs);
    else stream << "unknown request";
    stream << ")" << std::endl;

    myLogger->createMessage(
            MUST_ERROR_OVERLAPPED_RECV,
            pId,
            lId,
            MustErrorMessage,
            stream.str(),
            refs
            );
}

//=============================
// recvOverlapsRequests
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::recvOverlapsRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count)
{
	return irecvOverlapsRequests(pId, lId, datatype, buffer, count, 0, (MustRequestType)0 );
}


GTI_ANALYSIS_RETURN OverlapChecks::irecvOverlapsRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
	int hasRequest,
	MustRequestType request)
{
    if(buffer == MUST_IN_PLACE)
        return GTI_ANALYSIS_SUCCESS;
    if(buffer == MUST_BOTTOM)
        buffer = 0;
    I_Datatype * typeinfo = myDatMod->getDatatype(pId, datatype);
    if (typeinfo == NULL)
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    MustMemIntervalListType iList;
    if (! activeBlocks[pId2Rank(pId)].empty() || hasRequest )
    {
        iList = calcIntervalList(typeinfo, buffer, count, request, false);
    }
    else
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    GTI_ANALYSIS_RETURN res;
    if ( ! activeBlocks[pId2Rank(pId)].empty() )
        res = checkOverlapsRequests(pId, lId, iList, false, "The memory regions to be transfered by this receive operation overlap with regions spanned by a pending non-blocking operation!", MUST_ERROR_OVERLAPPED_RECV);
    if (hasRequest)
    {
        announceRequest(pId, lId, iList, request);
    }
    return res;
}

//=============================
// overlapsRequests
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::overlapsRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
        char isSend)
{
    if (isSend)
    {
        return sendOverlapsRequests(pId, lId, datatype, buffer, count);
    }
    else
    {
        return recvOverlapsRequests(pId, lId, datatype, buffer, count);
    }
}

//=============================
// announceSendRequest
//=============================
GTI_ANALYSIS_RETURN OverlapChecks::announceSendRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
        MustRequestType request)
{
    return announceRequest(pId, lId, datatype, buffer, count, true, request);
}

//=============================
// announceRecvRequest
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::announceRecvRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
        MustRequestType request)
{
    return announceRequest(pId, lId, datatype, buffer, count, false, request);
}

//=============================
// announceRequest
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::announceRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
        char isSend,
        MustRequestType request)
{
    if(buffer == MUST_IN_PLACE)
        return GTI_ANALYSIS_SUCCESS;
    if(buffer == MUST_BOTTOM)
        buffer = 0;
    I_Datatype * typeinfo = myDatMod->getDatatype(pId, datatype);
    if (typeinfo == NULL)
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    MustMemIntervalListType iList = calcIntervalList(typeinfo, buffer, count, request, isSend!=0);
    makeBlocksActive(pId, lId, iList, request);
    return GTI_ANALYSIS_SUCCESS;
}

GTI_ANALYSIS_RETURN OverlapChecks::announceRequest (
        MustParallelId pId,
        MustLocationId lId,
	MustMemIntervalListType iList,
        MustRequestType request)
{
    makeBlocksActive(pId, lId, iList, request);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// announcePSendRequest
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::announcePSendRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
        MustRequestType request)
{
    return announcePRequest(pId, lId, datatype, buffer, count, true, request);
}

//=============================
// announcePRecvRequest
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::announcePRecvRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
        MustRequestType request)
{
    return announcePRequest(pId, lId, datatype, buffer, count, false, request);
}

//=============================
// announcePRequest
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::announcePRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustDatatypeType datatype,
        MustAddressType buffer,
        int count,
        char isSend,
        MustRequestType request)
{
    if(buffer == MUST_IN_PLACE)
        return GTI_ANALYSIS_SUCCESS;
    if(buffer == MUST_BOTTOM)
        buffer = 0;
    I_Datatype * typeinfo = myDatMod->getDatatype(pId, datatype);
    if (typeinfo == NULL)
    {
        return GTI_ANALYSIS_SUCCESS;
    }
    preparedBlocklists[pId][request] = calcIntervalList(typeinfo, buffer, count, request, isSend!=0);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// prepared error message, given to the check as callback
//=============================

void OverlapChecks::outputStartPRequest(
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
    std::list<std::pair<MustParallelId,MustLocationId> > refs;
    std::stringstream stream;
    stream
        <<  "A start of a persistent request will transfer memory regions that are still used by a pending non-blocking communication!" << std::endl
        << "(Information on the request that is associated with the later communication:" << std::endl;
    I_Request* rInfo = myReqMod->getRequest (pId, request);
    if (rInfo) rInfo->printInfo (stream, &refs);
    else stream << "unknown request";
    stream << ")" << std::endl;

    myLogger->createMessage(
            MUST_ERROR_OVERLAPPED_SEND,
            pId,
            lId,
            MustErrorMessage,
            stream.str(),
            refs
            );
}

//=============================
// startPRequest
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::startPRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
//     GTI_ANALYSIS_RETURN ret = checkOverlapsRequests(pId, lId, preparedBlocklists[pId][request], preparedBlocklists[pId][request].begin()->isSend, &must::OverlapChecks::outputStartPRequest);
	//Check wether the request and PID are enlisted, if not this is likely an erroneous event in the first place
	mustPidRequestMap< MustMemIntervalListType >::iterator pIdMapPos = preparedBlocklists.find (pId2Rank(pId));
	if (pIdMapPos == preparedBlocklists.end ())
	{
		return GTI_ANALYSIS_SUCCESS;
        }
		
	std::map<MustRequestType, MustMemIntervalListType>::iterator reqMapPos = pIdMapPos->second.find (request);
    
    if (reqMapPos ==  pIdMapPos->second.end())
    {
    	return GTI_ANALYSIS_SUCCESS;
    }

    //if(preparedBlocklists[pId].count(request) < 1)
    //	return GTI_ANALYSIS_SUCCESS;

	GTI_ANALYSIS_RETURN ret = checkOverlapsRequests(pId, lId, reqMapPos->second, reqMapPos->second.begin()->isSend, "A start of a persistent request will transfer memory regions that are still used by a pending non-blocking communication!", MUST_ERROR_OVERLAPPED_SEND);
    makeBlocksActive(pId, lId, reqMapPos->second, request);
    return ret;
}

//=============================
// startPRequestArray
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::startPRequestArray (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType* requests,
        int count)
{
    int i;
    for (i=0; i<count; i++)
        startPRequest(pId, lId, requests[i]);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// finishRequest
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::finishRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
    makeBlocksInActive(pId, request);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// finishRequests
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::finishRequests (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType* request,
        int count)
{
    int i;
    for(i=0; i<count; i++)
        makeBlocksInActive(pId, request[i]);
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// freeRequest
//=============================

GTI_ANALYSIS_RETURN OverlapChecks::freeRequest (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType request)
{
    static bool firstRun=true;
    I_Request* rInfo;
    if ((firstRun) && ((rInfo = myReqMod->getRequest (pId, request)) != NULL) && (! rInfo->isPersistent()))
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;
        stream
            <<  "A non-persistent request handle is freed before end of transmission! There will be no further overlap checks for these requests." << std::endl
            << "(Information on the request that is associated with the later communication:" << std::endl;
        rInfo->printInfo (stream, &refs);
        stream << ")" << std::endl;

        myLogger->createMessage(
                MUST_INFO_FREE_NONPERSISTENT_REQUEST,
                pId,
                lId,
                MustInformationMessage,
                stream.str(),
                refs
                );
        firstRun=false;
    }
    makeBlocksInActive(pId, request);
    preparedBlocklists[pId].erase(request);
    return GTI_ANALYSIS_SUCCESS;
}


#ifdef DOT
//=============================
// generateOverlapHtml
//=============================
void OverlapChecks::generateOverlapHtml (std::string dotFile, std::string htmlFile, std::string imageFile)
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
            << "<title>MUST Overlapfile</title>" << std::endl
            << "<style type=\"text/css\">" << std::endl
            << "td,td,table {border:thin solid black}" << std::endl
            << "td.ee1{ background-color:#FFDDDD; text-align:center; vertical-align:middle;}" << std::endl
            << "td.ee2{ background-color:#FFEEEE; text-align:center; vertical-align:middle;}" << std::endl
            << "</style>" << std::endl
            << "</head>" << std::endl
            << "<body>" << std::endl
            << "<p> <b>MUST Overlap Details</b>, date: "
            << buf
            << "</p>" << std::endl
            << "<a href=\"" << MUST_OUTPUT_REDIR << "MUST_Output.html\" title=\"MUST error report\">Back to MUST error report</a><br>" << std::endl
            << "<table border=\"0\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">" << std::endl


            << "<tr>" << std::endl
            << "<td align=\"center\" bgcolor=\"#9999DD\" colspan=\"2\">" << std::endl
            << "<b>Message</b>"<< std::endl
            << "</td>" << std::endl
            << "</tr>" << std::endl
            << "<tr>" << std::endl
            << "<td class=\"ee2\" colspan=\"3\" >" << std::endl
            << "The application issued a set of MPI calls that overlap in communication buffers! " << std::endl
            << "The graph below shows details on this situation. " << std::endl
            << "The first colliding item of each involved communication request is highlighted." << std::endl
            << "</td>" << std::endl
            << "</tr>"  << std::endl

            << "<tr>" << std::endl
            << "<td align=\"center\" bgcolor=\"#7777BB\">"
            << "<b>Datatype Graph</b>"
            << "</td>" << std::endl
            << "</tr>" << std::endl
            << "<tr>" << std::endl
            << "<td class=\"ee2\" ><img src=\""<< MUST_OUTPUT_REDIR << imageFile <<"\" alt=\"deadlock\"></td>" << std::endl
            << "</tr>" << std::endl


            << "</table>" << std::endl
            << "</body>" << std::endl
            << "</html>" << std::endl;
    out.flush();
    out.close();
}
#endif

