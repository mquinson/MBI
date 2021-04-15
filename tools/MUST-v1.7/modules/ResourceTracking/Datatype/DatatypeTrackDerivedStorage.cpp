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
 * @file DatatypeTrackDerivedStorage.cpp
 *       @see MUST::DatatypeTrack.
 *
 *  @date 10.02.2011
 *  @author Tobias Hilbrich, Joachim Protze
 */

#include "GtiMacros.h"

#include <assert.h>
#include <sstream>
#include "DatatypeTrack.h"
#include "DatatypeTrackDerivedStorage.h"
#include "DatatypeTrackHelpers.h"

using namespace must;



//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullContiguousTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullContiguousTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    if (count == 0)
        return retval;
    MustTypemapType parentTypemap = parentInfos[0]->getTypemap(err);
    if (*err != 0)
        return retval;
    MustTypemapType::iterator iterType;
    int i;
    for (i=0; i<count; i++)
    {
        for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
        {
            retval.push_back(std::make_pair(iterType->first, iterType->second + parentInfos[0]->getExtent()*i));
        }
    }
    return retval;
}

//=============================
// getRealBlockList
//=============================
void FullContiguousTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    if (count == 0)
        return;
    const BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    MustAddressType pSize = parentInfos[0]->getSize();
    MustAddressType pExtent = parentInfos[0]->getExtent();
    myBlockInfo = buildStridedBlocklist(parentBlockInfo, pExtent, pSize, 0, 0, count, 0, 1);
    return;
}



//=============================
// Constructor
//=============================
FullContiguousTypeInfo::FullContiguousTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int count,
        std::vector<Datatype *> oldInfos,
        passDatatypeContiguousAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        count(count),
        myPassAcrossFunc (passAcrossFunc)
{
    lb = oldInfos[0]->getLb();
    extent = oldInfos[0]->getExtent() * count;
    true_lb = oldInfos[0]->getTrueLb();
    true_extent = extent - oldInfos[0]->getExtent() + oldInfos[0]->getTrueExtent();
    size = oldInfos[0]->getSize() * count;
    // no epsilon-magic needed, oldtype is already aligned
}

std::vector<struct posInfo> FullContiguousTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    MustAddressType parentsize = parentInfos[0]->getSize();
    MustAddressType parentextent = parentInfos[0]->getExtent();
    int nthBlock = errorpos / parentsize;
    ret.push_back(posInfo(nthBlock, count, getAddressVector(pos, nthBlock, parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthBlock, parentextent, MAXDOTSIBLINGS), "count"));
    errorpos -= nthBlock * parentsize;
    pos += nthBlock * parentsize;
    add += nthBlock * parentextent;
    return ret;
}

//=============================
// passAcross
//=============================
bool FullContiguousTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        count,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}


//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullVectorTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullVectorTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    if (count == 0 || blocklength == 0)
        return retval;
    MustTypemapType parentTypemap = parentInfos[0]->getTypemap(err);
    if (*err != 0)
        return retval;
    MustTypemapType::iterator iterType;
    int i, j;
    for (i=0; i<count; i++)
    {
        for (j=0; j<blocklength; j++)
        {
            for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
            {
                retval.push_back(std::make_pair(iterType->first, iterType->second + parentInfos[0]->getExtent() * (i * stride + j)));
            }
        }
    }
    return retval;
}


//=============================
// getRealBlockInfo
//=============================
void FullVectorTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    if (count == 0)
        return;
    const BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    MustAddressType pSize = parentInfos[0]->getSize();
    MustAddressType pExtent = parentInfos[0]->getExtent();
    myBlockInfo = buildStridedBlocklist(parentBlockInfo, pExtent, pSize, 0, 0, blocklength, stride*pExtent, count);
    return;
}



//=============================
// Constructor
//=============================
FullVectorTypeInfo::FullVectorTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int count,
        int blocklength,
        int stride,
        std::vector<Datatype *> oldInfos,
        passDatatypeVectorAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        count(count),
        blocklength(blocklength),
        stride(stride),
        myPassAcrossFunc (passAcrossFunc)
{
    if (stride < 0){ // blocks are placed "left of" first block
        lb = stride * (count-1) * oldInfos[0]->getExtent() + oldInfos[0]->getLb();
        extent = ((-stride) * (count-1) + blocklength) * oldInfos[0]->getExtent();
    }else{
        lb = oldInfos[0]->getLb();
        extent = (stride * (count-1) + blocklength) * oldInfos[0]->getExtent();
    }
    size = oldInfos[0]->getSize() * blocklength * count;
    true_lb = lb - oldInfos[0]->getLb() + oldInfos[0]->getTrueLb();
    true_extent = extent - oldInfos[0]->getExtent() + oldInfos[0]->getTrueExtent();
    // no epsilon-magic needed, oldtype is already aligned -- we are using just multiples of oldtype
}

std::vector<struct posInfo> FullVectorTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    MustAddressType parentsize = parentInfos[0]->getSize();
    MustAddressType parentextent = parentInfos[0]->getExtent();
    int nthBlock = errorpos / (parentsize * blocklength);
    ret.push_back(posInfo(nthBlock, count, getAddressVector(pos, nthBlock, parentsize*blocklength, MAXDOTSIBLINGS), getAddressVector(add, nthBlock, parentextent * stride, MAXDOTSIBLINGS), "count"));
    errorpos -= nthBlock * (parentsize * blocklength);
    pos += nthBlock * (parentsize * blocklength);
    add += nthBlock * parentextent * stride;
    int nthElem = errorpos / parentsize;
    ret.push_back(posInfo(nthElem, blocklength, getAddressVector(pos, nthElem, parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthElem, parentextent, MAXDOTSIBLINGS), "blocklength"));
    errorpos -= nthElem * parentsize;
    pos += nthElem * parentsize;
    add += nthElem * parentextent;
    return ret;
}


//=============================
// passAcross
//=============================
bool FullVectorTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        count,
        blocklength,
        stride,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullHVectorTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullHVectorTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    if (count == 0 || blocklength == 0)
        return retval;
    MustTypemapType parentTypemap = parentInfos[0]->getTypemap(err);
    if (*err != 0)
        return retval;
    MustTypemapType::iterator iterType;
    int i, j;
    for (i=0; i<count; i++)
    {
        for (j=0; j<blocklength; j++)
        {
            for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
            {
                retval.push_back(std::make_pair(iterType->first, iterType->second + i * stride + parentInfos[0]->getExtent() * j));
            }
        }
    }
    return retval;
}


//=============================
// getRealBlockInfo
//=============================
void FullHVectorTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    if (count == 0)
        return;
    const BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    MustAddressType pSize = parentInfos[0]->getSize();
    MustAddressType pExtent = parentInfos[0]->getExtent();
    myBlockInfo = buildStridedBlocklist(parentBlockInfo, pExtent, pSize, 0, 0, blocklength, stride, count);
    return;
}

//=============================
// checkAlignment
//=============================
MustAddressType FullHVectorTypeInfo::checkAlignment(void) const
{
    for (int i=0; i<count; i++)
    {
        if( (stride % parentInfos[0]->getAlignment()) != 0 )
            return stride;
    }
    return 0;
}


//=============================
// Constructor
//=============================
FullHVectorTypeInfo::FullHVectorTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int count,
        int blocklength,
        MustAddressType stride,
        std::vector<Datatype *> oldInfos,
        passDatatypeHvectorAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        count(count),
        blocklength(blocklength),
        stride(stride),
        myPassAcrossFunc (passAcrossFunc)
{
    if (stride < 0){ // blocks are placed "left of" first block
        lb = stride * (count-1) + oldInfos[0]->getLb();
        extent = (-stride) * (count-1) + blocklength * oldInfos[0]->getExtent();
    }else{
        lb = oldInfos[0]->getLb();
        extent = stride * (count-1) + blocklength * oldInfos[0]->getExtent();
    }
    size = oldInfos[0]->getSize() * blocklength * count;
    true_lb = lb - oldInfos[0]->getLb() + oldInfos[0]->getTrueLb();
    true_extent = extent - oldInfos[0]->getExtent() + oldInfos[0]->getTrueExtent();
    epsilonMagic();
}

std::vector<struct posInfo> FullHVectorTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    MustAddressType parentsize = parentInfos[0]->getSize();
    MustAddressType parentextent = parentInfos[0]->getExtent();
    int nthBlock = errorpos / (parentsize * blocklength);
    ret.push_back(posInfo(nthBlock, count, getAddressVector(pos, nthBlock, parentsize * blocklength, MAXDOTSIBLINGS), getAddressVector(add, nthBlock, stride, MAXDOTSIBLINGS), "count"));
    errorpos -= nthBlock * (parentsize * blocklength);
    pos += nthBlock * (parentsize * blocklength);
    add += nthBlock * stride;
    int nthElem = errorpos / parentsize;
    ret.push_back(posInfo(nthElem, blocklength, getAddressVector(pos, nthElem, parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthElem, parentextent, MAXDOTSIBLINGS), "blocklength"));
    errorpos -= nthElem * parentsize;
    pos += nthElem * parentsize;
    add += nthElem * parentextent;
    return ret;
}


//=============================
// passAcross
//=============================
bool FullHVectorTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        count,
        blocklength,
        stride,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}


//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullIndexedTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Destructor
//=============================
FullIndexedTypeInfo::~FullIndexedTypeInfo(void)
{
    if (arrayOfBlocklengths)
        delete [] arrayOfBlocklengths;
    if (arrayOfDisplacements)
        delete [] arrayOfDisplacements;
}

//=============================
// Constructor
//=============================
FullIndexedTypeInfo::FullIndexedTypeInfo(void)
    : count (0),
      arrayOfBlocklengths (NULL),
      arrayOfDisplacements (NULL),
      myPassAcrossFunc (NULL)
{

}

//=============================
// Constructor
//=============================
FullIndexedTypeInfo::FullIndexedTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int count,
        const int* blocklengths,
        const int* displacements,
        std::vector<Datatype *> oldInfos,
        passDatatypeIndexedAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        count(count),
        myPassAcrossFunc (passAcrossFunc)
{
    arrayOfBlocklengths = new int[count] ();
    arrayOfDisplacements = new int[count] ();
    memcpy(arrayOfBlocklengths, blocklengths, count*sizeof(int));
    memcpy(arrayOfDisplacements, displacements, count*sizeof(int));
    MustAddressType ub;

    // Calculate Sizes
    lb = MUST_ADDR_INFTY;
    ub = MUST_ADDR_NEG_INFTY;
    size = 0;
    int i;
    for (i=0; i<count; i++)
    {
        if (arrayOfBlocklengths[i]==0)
            continue;
        if (arrayOfDisplacements[i] < lb) // find lowest displacement / min(D)
            lb = arrayOfDisplacements[i];
        if (arrayOfDisplacements[i] + arrayOfBlocklengths[i] > ub) // find max(D+B)
            ub = arrayOfDisplacements[i] + arrayOfBlocklengths[i];
        size += arrayOfBlocklengths[i]; // count oldtypes
    }
    lb *= oldInfos[0]->getExtent(); //Move lb to bytes
    extent = ub * oldInfos[0]->getExtent() - lb; // max(D+B) * extent - min(D)
    lb += oldInfos[0]->getLb(); // add lb of parent to lowest displacement
    true_lb = lb - oldInfos[0]->getLb() + oldInfos[0]->getTrueLb();
    true_extent = extent - oldInfos[0]->getExtent() + oldInfos[0]->getTrueExtent();
    size *= oldInfos[0]->getSize();
    // no epsilon-magic needed, oldtype is already aligned -- we are using just multiples of oldtype
}


//=============================
// Copy-Constructor
//=============================
FullIndexedTypeInfo::FullIndexedTypeInfo(FullIndexedTypeInfo& info)
{
    arrayOfBlocklengths = new int[info.count] ();
    arrayOfDisplacements = new int[info.count] ();
    memcpy(arrayOfBlocklengths, info.arrayOfBlocklengths, info.count*sizeof(int));
    memcpy(arrayOfDisplacements, info.arrayOfDisplacements, info.count*sizeof(int));
    count=info.count;
    myPassAcrossFunc = info.myPassAcrossFunc;
}

//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullIndexedTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    if (count == 0)
        return retval;
    MustTypemapType parentTypemap = parentInfos[0]->getTypemap(err);
    if (*err != 0)
        return retval;
    MustTypemapType::iterator iterType;
    int i, j;
    for (i=0; i<count; i++)
    {
        for (j=0; j<arrayOfBlocklengths[i]; j++)
        {
            for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
            {
                retval.push_back(std::make_pair(iterType->first, iterType->second + (arrayOfDisplacements[i] + j) * parentInfos[0]->getExtent()));
            }
        }
    }
    return retval;
}

//=============================
// getRealBlockInfo
//=============================
void FullIndexedTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    if (count == 0)
        return;
    const BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    MustAddressType pSize = parentInfos[0]->getSize();
    MustAddressType pExtent = parentInfos[0]->getExtent(),
                    pos = 0;
    if(parentBlockInfo.overlapped)
    {
        myBlockInfo.overlapped=parentBlockInfo.overlapped;
        myBlockInfo.posA = parentBlockInfo.posA;
        myBlockInfo.posB = parentBlockInfo.posB;
    }
    MustStridedBlocklistType tempList;
    for (int i=0; i<count; i++)
    {
        tempList = buildStridedBlocklist(parentBlockInfo, pExtent, pSize, arrayOfDisplacements[i] * pExtent, pos, arrayOfBlocklengths[i], 0, 1);
        myBlockInfo.insert(tempList.begin(), tempList.end());
        pos += arrayOfBlocklengths[i]*pSize;
    }
    return;
}


std::vector<struct posInfo> FullIndexedTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    std::vector<MustAddressType> addV, posV;
    addV.push_back(add);
    posV.push_back(pos);
    MustAddressType parentsize = parentInfos[0]->getSize(),
                    parentextent = parentInfos[0]->getExtent();
    int nthBlock = 0;
    for (; errorpos >= parentsize * arrayOfBlocklengths[nthBlock]; ++nthBlock)
    {
        errorpos -= parentsize * arrayOfBlocklengths[nthBlock];
        pos += parentsize * arrayOfBlocklengths[nthBlock];
    }
    int nthElem = errorpos / parentsize;
    MustAddressType sibPos=pos+ parentsize * arrayOfBlocklengths[nthBlock];
    for (int j=1, i=nthBlock+1; j<MAXDOTSIBLINGS && i<count; j++, i++)
    {
        addV.push_back(add + arrayOfDisplacements[i] * parentextent);
        posV.push_back(sibPos);
        sibPos += parentsize * arrayOfBlocklengths[i];
    }
    add += arrayOfDisplacements[nthBlock] * parentextent;
    ret.push_back(posInfo(nthBlock, count, posV, addV, "count"));
    ret.push_back(posInfo(nthElem, arrayOfBlocklengths[nthBlock], getAddressVector(pos, nthElem , parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthElem, parentextent, MAXDOTSIBLINGS), "blocklength"));
    errorpos -= nthElem * parentsize;
    pos += nthElem * parentsize;
    add += nthElem * parentextent;
    return ret;
}


//=============================
// passAcross
//=============================
bool FullIndexedTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        count,
        arrayOfBlocklengths,
        arrayOfDisplacements,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullHIndexedTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Destructor
//=============================
FullHIndexedTypeInfo::~FullHIndexedTypeInfo(void)
{
    if (arrayOfBlocklengths)
        delete [] arrayOfBlocklengths;
    if (arrayOfDisplacements)
        delete [] arrayOfDisplacements;
}

//=============================
// Constructor
//=============================
FullHIndexedTypeInfo::FullHIndexedTypeInfo(void)
 :  count (0),
    arrayOfBlocklengths (NULL),
    arrayOfDisplacements (NULL),
    myPassAcrossFunc (NULL)
{

}

//=============================
// Constructor
//=============================
FullHIndexedTypeInfo::FullHIndexedTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int count,
        const int* blocklengths,
        const MustAddressType* displacements,
        std::vector<Datatype *> oldInfos,
        passDatatypeHindexedAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        count(count),
        myPassAcrossFunc (passAcrossFunc)
{
    arrayOfBlocklengths = new int[count] ();
    arrayOfDisplacements = new MustAddressType[count] ();
    memcpy(arrayOfBlocklengths, blocklengths, count*sizeof(int));
    memcpy(arrayOfDisplacements, displacements, count*sizeof(MustAddressType));
    MustAddressType ub;

    // Calculate Sizes
    lb = MUST_ADDR_INFTY;
    ub = MUST_ADDR_NEG_INFTY;
    size = 0;
    int i;
    for (i=0; i<count; i++)
    {
        if (arrayOfBlocklengths[i]==0)
            continue;
        if (arrayOfDisplacements[i] < lb) // find lowest displacement / min(D)
            lb = arrayOfDisplacements[i];
        if (arrayOfDisplacements[i] + arrayOfBlocklengths[i] * oldInfos[0]->getExtent() > ub) // find max(D+B)
            ub = arrayOfDisplacements[i] + arrayOfBlocklengths[i] * oldInfos[0]->getExtent();
        size += arrayOfBlocklengths[i]; // count oldtypes
    }
    extent = ub - lb; // max(D+B) * extent - min(D)
    lb += oldInfos[0]->getLb(); // add lb of parent to lowest displacement
    size *= oldInfos[0]->getSize();
    true_lb = lb - oldInfos[0]->getLb() + oldInfos[0]->getTrueLb();
    true_extent = extent - oldInfos[0]->getExtent() + oldInfos[0]->getTrueExtent();
    epsilonMagic();
}

//=============================
// Copy-Constructor
//=============================
FullHIndexedTypeInfo::FullHIndexedTypeInfo(FullHIndexedTypeInfo& info)
{
    arrayOfBlocklengths = new int[info.count] ();
    arrayOfDisplacements = new MustAddressType[info.count] ();
    memcpy(arrayOfBlocklengths, info.arrayOfBlocklengths, info.count*sizeof(int));
    memcpy(arrayOfDisplacements, info.arrayOfDisplacements, info.count*sizeof(MustAddressType));
    count=info.count;
    myPassAcrossFunc = info.myPassAcrossFunc;
}

//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullHIndexedTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    if (count == 0)
        return retval;
    MustTypemapType parentTypemap = parentInfos[0]->getTypemap(err);
    if (*err != 0)
        return retval;
    MustTypemapType::iterator iterType;
    int i, j;
    for (i=0; i<count; i++)
    {
        for (j=0; j<arrayOfBlocklengths[i]; j++)
        {
            for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
            {
                retval.push_back(std::make_pair(iterType->first, iterType->second + arrayOfDisplacements[i] + j * parentInfos[0]->getExtent()));
            }
        }
    }
    return retval;
}

//=============================
// checkAlignment
//=============================
MustAddressType FullHIndexedTypeInfo::checkAlignment(void) const
{
    for (int i=0; i<count; i++)
    {
        if( (arrayOfDisplacements[i] % parentInfos[i]->getAlignment()) != 0 )
            return arrayOfDisplacements[i];
    }
    return 0;
}

//=============================
// getRealBlockInfo
//=============================
void FullHIndexedTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    if (count == 0)
        return;
    const BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    MustAddressType pSize = parentInfos[0]->getSize();
    MustAddressType pExtent = parentInfos[0]->getExtent(),
                    pos = 0;
    if(parentBlockInfo.overlapped)
    {
        myBlockInfo.overlapped=parentBlockInfo.overlapped;
        myBlockInfo.posA = parentBlockInfo.posA;
        myBlockInfo.posB = parentBlockInfo.posB;
    }
    MustStridedBlocklistType tempList;
    for (int i=0; i<count; i++)
    {
        tempList = buildStridedBlocklist(parentBlockInfo, pExtent, pSize, arrayOfDisplacements[i], pos, arrayOfBlocklengths[i], 0, 1);
        myBlockInfo.insert(tempList.begin(), tempList.end());
        pos += arrayOfBlocklengths[i]*pSize;
    }
    return;
}



std::vector<struct posInfo> FullHIndexedTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    std::vector<MustAddressType> addV, posV;
    addV.push_back(add);
    posV.push_back(pos);
    MustAddressType parentsize = parentInfos[0]->getSize();
    MustAddressType parentextent = parentInfos[0]->getExtent();
    int nthBlock = 0;
    for (; errorpos >= parentsize * arrayOfBlocklengths[nthBlock]; ++nthBlock)
    {
        errorpos -= parentsize * arrayOfBlocklengths[nthBlock];
        pos += parentsize * arrayOfBlocklengths[nthBlock];
    }
    int nthElem = errorpos / parentsize;
    MustAddressType sibPos=pos + parentsize * arrayOfBlocklengths[nthBlock];
    for (int j=1, i=nthBlock+1; j<MAXDOTSIBLINGS && i<count; j++, i++)
    {
        addV.push_back(add + arrayOfDisplacements[i]);
        posV.push_back(sibPos);
        sibPos += parentsize * arrayOfBlocklengths[i];
    }
    add += arrayOfDisplacements[nthBlock];
    ret.push_back(posInfo(nthBlock, count, posV, addV, "count"));
    ret.push_back(posInfo(nthElem, arrayOfBlocklengths[nthBlock], getAddressVector(pos, nthElem, parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthElem, parentextent, MAXDOTSIBLINGS), "blocklength"));
    errorpos -= nthElem * parentsize;
    pos += nthElem * parentsize;
    add += nthElem * parentextent;
    return ret;
}


//=============================
// passAcross
//=============================
bool FullHIndexedTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        count,
        arrayOfBlocklengths,
        arrayOfDisplacements,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}


//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullStructTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Destructor
//=============================
FullStructTypeInfo::~FullStructTypeInfo(void)
{
    if (arrayOfBlocklengths)
        delete [] arrayOfBlocklengths;
    if (arrayOfDisplacements)
        delete [] arrayOfDisplacements;
}

//=============================
// Constructor
//=============================
FullStructTypeInfo::FullStructTypeInfo(void)
 : count (0),
   arrayOfBlocklengths (NULL),
   arrayOfDisplacements (NULL),
   myPassAcrossFunc (NULL)
{

}

//=============================
// Constructor
//=============================
FullStructTypeInfo::FullStructTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int count,
        const int* blocklengths,
        const MustAddressType* displacements,
        std::vector<Datatype *> oldInfos,
        passDatatypeStructAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        count(count),
        myPassAcrossFunc (passAcrossFunc)
{
    arrayOfBlocklengths = new int[count] ();
    arrayOfDisplacements = new MustAddressType[count] ();
    memcpy(arrayOfBlocklengths, blocklengths, count*sizeof(int));
    memcpy(arrayOfDisplacements, displacements, count*sizeof(MustAddressType));

    // Calculate Sizes
    bool isExplicitLb = false, isExplicitUb = false;
    MustAddressType explicitLb, explicitUb;
    MustAddressType true_ub, ub;

    if (arrayOfBlocklengths[0]>0)
    {
        ub = arrayOfDisplacements[0] + arrayOfBlocklengths[0] * oldInfos[0]->getExtent() + oldInfos[0]->getLb();
        // init with first values;
        lb = arrayOfDisplacements[0] + oldInfos[0]->getLb();
    }else{
        ub = MUST_ADDR_NEG_INFTY; // - infinity
        lb = MUST_ADDR_INFTY; // + infinity, without compiler warning!
    }
    // Need to test for basic boundmarkers to not to count them
    if (!(oldInfos[0]->isPredefined() && ( oldInfos[0]->hasExplicitLb() || oldInfos[0]->hasExplicitUb() ) && arrayOfBlocklengths[0]>0 ))
    {
        true_lb = arrayOfDisplacements[0] + oldInfos[0]->getTrueLb();
        true_ub = arrayOfDisplacements[0] + (arrayOfBlocklengths[0]-1) * oldInfos[0]->getExtent() + oldInfos[0]->getTrueExtent() + oldInfos[0]->getTrueLb();
    }
    else
    {
        true_lb = MUST_ADDR_INFTY; // + infinity, without compiler warning!
        true_ub = MUST_ADDR_NEG_INFTY; // - infinity
    }
    // Need to test for basic boundmarkers to not to count them
    size = 0;
    int i;

    for (i = 0; i < count; i++)
    {
        if (arrayOfBlocklengths[i]==0)
            continue;
        myIsC &= oldInfos[i]->isC(); // true if all parents true!
        myIsFortran &= oldInfos[i]->isFortran(); // true if all parents true!
        if (alignment < oldInfos[i]->getAlignment()) // find max(alignment)
            alignment = oldInfos[i]->getAlignment();
        size += arrayOfBlocklengths[i] * oldInfos[i]->getSize(); // count oldtypes
        // lb, ub & extent
        if (oldInfos[i]->hasExplicitLb())
        {
            myHasExplicitLb = true;
            if (!isExplicitLb)
            { // first found MPI_LB
                isExplicitLb = true;
                explicitLb = arrayOfDisplacements[i]; // init with displacement of first marker
            }
            else if (explicitLb > arrayOfDisplacements[i])
            {
                explicitLb = arrayOfDisplacements[i]; // min(displ) of LB-markers
            }
        }
        else if (oldInfos[i]->hasExplicitUb())
        { 
            myHasExplicitUb = true;
            if (!isExplicitUb)
            { // first found MPI_UB
                isExplicitUb = true;
                explicitUb = arrayOfDisplacements[i] + arrayOfBlocklengths[i] * oldInfos[i]->getExtent(); // init with displacement of first marker
            }
            else if (explicitUb < arrayOfDisplacements[i] + arrayOfBlocklengths[i] * oldInfos[i]->getExtent())
            {
                explicitUb = arrayOfDisplacements[i] + arrayOfBlocklengths[i] * oldInfos[i]->getExtent(); // max(displ) of UB-markers
            }
        }

        if (lb > arrayOfDisplacements[i] + oldInfos[i]->getLb()) // find lowest displacement / min(D)
            lb = arrayOfDisplacements[i] + oldInfos[i]->getLb();
        // Need to test for basic boundmarkers to not to count them
        if (!(oldInfos[i]->isPredefined() && ( oldInfos[i]->hasExplicitLb() || oldInfos[i]->hasExplicitUb() )) && 
            true_lb > arrayOfDisplacements[i] + oldInfos[i]->getTrueLb()) // find lowest displacement / min(D)
            true_lb = arrayOfDisplacements[i] + oldInfos[i]->getTrueLb();
        if (ub < arrayOfDisplacements[i] + arrayOfBlocklengths[i] * oldInfos[i]->getExtent() + oldInfos[i]->getLb()) // find max(D + B * extent - lb)
        {
            ub = arrayOfDisplacements[i] + arrayOfBlocklengths[i] * oldInfos[i]->getExtent() + oldInfos[i]->getLb();
            epsilon = oldInfos[i]->getEpsilon(); // save the epsilon of the rightmost block/datatype
        }
        // Need to test for basic boundmarkers to not to count them
        if (!(oldInfos[i]->isPredefined() && ( oldInfos[i]->hasExplicitLb() || oldInfos[i]->hasExplicitUb() )) && 
            true_ub < arrayOfDisplacements[i] + (arrayOfBlocklengths[i]-1) * oldInfos[i]->getExtent() + oldInfos[i]->getTrueExtent() + oldInfos[i]->getTrueLb()) // find max(D + B * extent - lb)
        {
            true_ub = arrayOfDisplacements[i] + (arrayOfBlocklengths[i]-1) * oldInfos[i]->getExtent() + oldInfos[i]->getTrueExtent() + oldInfos[i]->getTrueLb();
        }
    }
    true_extent = true_ub - true_lb;

    if (isExplicitLb)
        lb = explicitLb;
    if (isExplicitUb)
    {
        extent = explicitUb - lb; // ub - min(D)
        epsilon = 0;
        // no epsilon with explicit MPI_UB -- see MPI-standard-2.2 sect 4.1.6
    }
    else
    {
        extent = ub - lb; // max(D + B * extent) - min(D)
        // epsilon of the rightmost block is saved to this->getEpsilon(). epsilonmagic removes it from extent and calculates a new one.
        epsilonMagic();
    }

}

//=============================
// Copy-Constructor
//=============================
FullStructTypeInfo::FullStructTypeInfo(FullStructTypeInfo& info)
{
    arrayOfBlocklengths = new int[info.count] ();
    arrayOfDisplacements = new MustAddressType[info.count] ();
    memcpy(arrayOfBlocklengths, info.arrayOfBlocklengths, info.count*sizeof(int));
    memcpy(arrayOfDisplacements, info.arrayOfDisplacements, info.count*sizeof(MustAddressType));
    count=info.count;
    myPassAcrossFunc = info.myPassAcrossFunc;
}


//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullStructTypeInfo::getFullTypemap(int* err)
{
    int i, j;
    MustTypemapType retval;
    if (count == 0)
        return retval;
    MustTypemapType parentTypemap;
    MustTypemapType::iterator iterType;
    for (i=0; i<count; i++)
    {
        if (arrayOfBlocklengths[i]==0){
            continue;
        }
        parentTypemap = parentInfos[i]->getTypemap(err);
        if (*err != 0)
        {
            return retval;
        }
        for (j=0; j<arrayOfBlocklengths[i]; j++)
        {
            for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
            {
                retval.push_back(std::make_pair(iterType->first, iterType->second + arrayOfDisplacements[i] + j * parentInfos[i]->getExtent()));
            }
        }
    }
    return retval;
}

//=============================
// checkAlignment
//=============================
MustAddressType FullStructTypeInfo::checkAlignment(void) const
{
    for (int i=0; i<count; i++)
    {
        if( (arrayOfDisplacements[i] % parentInfos[i]->getAlignment()) != 0 )
            return arrayOfDisplacements[i];
    }
    return 0;
}

//=============================
// getRealBlockInfo
//=============================
void FullStructTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    if (count == 0)
        return;
    MustAddressType pSize, pExtent, pos = 0;
    MustStridedBlocklistType tempList;
    for (int i=0; i<count; i++)
    {
        const BlockInfo& parentBlockInfo = parentInfos[i]->getBlockInfo();
//         myBlockInfo.overlapped = myBlockInfo.overlapped || parentBlockInfo.overlapped;
        if(!myBlockInfo.overlapped && parentBlockInfo.overlapped)
        {
            myBlockInfo.overlapped = parentBlockInfo.overlapped;
            myBlockInfo.posA = pos + parentBlockInfo.posA;
            myBlockInfo.posB = pos + parentBlockInfo.posB;
        }
        pSize = parentInfos[i]->getSize();
        pExtent = parentInfos[i]->getExtent();
        tempList = buildStridedBlocklist(parentBlockInfo, pExtent, pSize, arrayOfDisplacements[i], pos, arrayOfBlocklengths[i], 0, 1);
        myBlockInfo.insert(tempList.begin(), tempList.end());
        pos += arrayOfBlocklengths[i]*pSize;
    }
    return;
}

std::vector<struct posInfo> FullStructTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    std::vector<MustAddressType> addV, posV;
    addV.push_back(add);
    posV.push_back(pos);
    int nthBlock = 0;
    for (; errorpos >= parentInfos[nthBlock]->getSize() * arrayOfBlocklengths[nthBlock]; ++nthBlock)
    {
        errorpos -= parentInfos[nthBlock]->getSize() * arrayOfBlocklengths[nthBlock];
        pos += parentInfos[nthBlock]->getSize() * arrayOfBlocklengths[nthBlock];
    }
    int nthElem=0;
    MustAddressType parentsize = parentInfos[nthBlock]->getSize();
    MustAddressType parentextent = parentInfos[nthBlock]->getExtent();
    if (errorpos > 0)
        nthElem = errorpos / parentsize;
    MustAddressType sibPos=pos + parentsize * arrayOfBlocklengths[nthBlock];
    for (int j=1, i=nthBlock+1; j<MAXDOTSIBLINGS && i<count; j++, i++)
    {
        addV.push_back(add + arrayOfDisplacements[i]);
        posV.push_back(sibPos);
        sibPos += parentInfos[i]->getSize() * arrayOfBlocklengths[i];
    }
    add += arrayOfDisplacements[nthBlock];
    ret.push_back(posInfo(nthBlock, count, posV, addV, "count"));
    ret.push_back(posInfo(nthElem, arrayOfBlocklengths[nthBlock], getAddressVector(pos, nthElem, parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthElem, parentextent, MAXDOTSIBLINGS), "blocklength"));
    errorpos -= nthElem * parentsize;
    pos += nthElem * parentsize;
    add += nthElem * parentextent;
    return ret;
}

//=============================
// passAcross
//=============================
bool FullStructTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType *baseTypeRemoteIds = NULL;

    if (count > 0)
    {
        baseTypeRemoteIds = new MustRemoteIdType[count];

        for (std::vector<Datatype *>::size_type i = 0; i < parentInfos.size(); i++)
        {
            if (parentInfos[i])
                baseTypeRemoteIds[i] = parentInfos[i]->getRemoteId();
            else
                baseTypeRemoteIds[i] = 0;
        }
    }

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        count,
        arrayOfBlocklengths,
        arrayOfDisplacements,
        baseTypeRemoteIds,
        //
        toPlaceId
        );

    if (baseTypeRemoteIds)
        delete [] baseTypeRemoteIds;

    return true;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullIndexedBlockTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Destructor
//=============================
FullIndexedBlockTypeInfo::~FullIndexedBlockTypeInfo(void)
{
    if (arrayOfDisplacements)
        delete [] arrayOfDisplacements;
}

//=============================
// Constructor
//=============================
FullIndexedBlockTypeInfo::FullIndexedBlockTypeInfo(void)
 : count (0),
   blocklength (0),
   arrayOfDisplacements (NULL),
   myPassAcrossFunc (NULL)
{

}

//=============================
// Constructor
//=============================
FullIndexedBlockTypeInfo::FullIndexedBlockTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int count,
        int blocklength,
        const int* displacements,
        std::vector<Datatype *> oldInfos,
        passDatatypeIndexedBlockAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        count(count),
        blocklength(blocklength),
        myPassAcrossFunc (passAcrossFunc)
{
    arrayOfDisplacements = new int[count] ();
    memcpy(arrayOfDisplacements, displacements, count*sizeof(int));

    // Calculate Sizes
    lb = arrayOfDisplacements[0];
    MustAddressType ub = arrayOfDisplacements[0];
    int i;
    for (i=1; i<count; i++)
    {
        if (arrayOfDisplacements[i] < lb) // find lowest displacement / min(D)
            lb = arrayOfDisplacements[i];
        if (arrayOfDisplacements[i] > ub) // find max(D)
            ub = arrayOfDisplacements[i];
    }
    lb *= oldInfos[0]->getExtent(); //Move lb to bytes
    extent = (ub + blocklength) * oldInfos[0]->getExtent() - lb; // (max(D) + b) * extent - min(D) * extent
    lb += oldInfos[0]->getLb(); // add lb of parent to lowest displacement
    size = oldInfos[0]->getSize() * blocklength * count;
    true_lb = lb - oldInfos[0]->getLb() + oldInfos[0]->getTrueLb();
    true_extent = extent - oldInfos[0]->getExtent() + oldInfos[0]->getTrueExtent();
    // no epsilon-magic needed, oldtype is already aligned -- we are using just multiples of oldtype
}


//=============================
// Copy-Constructor
//=============================
FullIndexedBlockTypeInfo::FullIndexedBlockTypeInfo(FullIndexedBlockTypeInfo& info)
{
    arrayOfDisplacements = new int[info.count] ();
    memcpy(arrayOfDisplacements, info.arrayOfDisplacements, info.count*sizeof(int));
    blocklength=info.blocklength;
    count=info.count;
    myPassAcrossFunc = info.myPassAcrossFunc;
}


//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullIndexedBlockTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    if (count == 0 || blocklength == 0)
        return retval;
    MustTypemapType parentTypemap = parentInfos[0]->getTypemap(err);
    if (*err != 0)
        return retval;
    MustTypemapType::iterator iterType;
    int i, j;
    for (i=0; i<count; i++)
    {
        for (j=0; j<blocklength; j++)
        {
            for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
            {
                retval.push_back(std::make_pair(iterType->first, iterType->second + (arrayOfDisplacements[i] + j) * parentInfos[0]->getExtent()));
            }
        }
    }
    return retval;
}

//=============================
// getRealBlockInfo
//=============================
void FullIndexedBlockTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    if (count == 0)
        return;
    const BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    MustAddressType pSize = parentInfos[0]->getSize();
    MustAddressType pExtent = parentInfos[0]->getExtent();
    if(parentBlockInfo.overlapped)
    {
        myBlockInfo.overlapped=parentBlockInfo.overlapped;
        myBlockInfo.posA = parentBlockInfo.posA;
        myBlockInfo.posB = parentBlockInfo.posB;
    }
    MustStridedBlocklistType tempList;
    for (int i=0; i<count; i++)
    {
        tempList = buildStridedBlocklist(parentBlockInfo, pExtent, pSize, arrayOfDisplacements[i] * pExtent, i * blocklength * pSize, blocklength, 0, 1);
        myBlockInfo.insert(tempList.begin(), tempList.end());
    }
    return;
}




std::vector<struct posInfo> FullIndexedBlockTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    std::vector<MustAddressType> addV, posV;
    addV.push_back(add);
    posV.push_back(pos);
    MustAddressType parentsize = parentInfos[0]->getSize();
    MustAddressType parentextent = parentInfos[0]->getExtent();
    int nthBlock = errorpos / (parentsize * blocklength);
    errorpos -= nthBlock * (parentsize * blocklength);
    pos += nthBlock * (parentsize * blocklength);
    int nthElem = errorpos / parentsize;
    for (int j=1, i=nthBlock+1; j<MAXDOTSIBLINGS && i<count; j++, i++)
    {
        addV.push_back(add + arrayOfDisplacements[i] * parentextent);
        posV.push_back(pos - j*parentsize * blocklength);
    }
    add += arrayOfDisplacements[nthBlock] * parentextent;
    ret.push_back(posInfo(nthBlock, count, posV, addV, "count"));
    ret.push_back(posInfo(nthElem, blocklength, getAddressVector(pos, nthElem, parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthElem, parentextent, MAXDOTSIBLINGS), "blocklength"));
    errorpos -= nthElem * parentsize;
    pos += nthElem * parentsize;
    add += nthElem * parentextent;
    return ret;
}


//=============================
// passAcross
//=============================
bool FullIndexedBlockTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        count,
        blocklength,
        arrayOfDisplacements,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}



//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullResizedTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullResizedTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    retval = parentInfos[0]->getTypemap(err);
    if (parentInfos[0]->hasExplicitLb() || parentInfos[0]->hasExplicitUb())
    { // there are boundmarkers to be striped off before
        MustAddressType lUb, lLb;
        stripBoundmarkersFromTypemap(retval, lUb, lLb);
    }
    retval.push_front(std::make_pair(MUST_MPI_LB, lb));
    retval.push_back(std::make_pair(MUST_MPI_UB, lb+extent));
    return retval;
}

//=============================
// getRealBlockInfo
//=============================
void FullResizedTypeInfo::getRealBlockInfo()
{
    BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    myBlockInfo = parentBlockInfo;
}


//=============================
// Constructor
//=============================
FullResizedTypeInfo::FullResizedTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        MustAddressType newLb,
        MustAddressType newExtent,
        std::vector<Datatype *> oldInfos,
        passDatatypeResizedAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        myPassAcrossFunc (passAcrossFunc)
/*        lb(lb),
        extent(extent), */
{

    // Calculate Sizes
    true_lb = oldInfos[0]->getTrueLb();
    true_extent = oldInfos[0]->getTrueExtent();
    size = oldInfos[0]->getSize();
    lb = newLb;
    extent = newExtent;
    myHasExplicitLb = true;
    myHasExplicitUb = true;
    epsilon = 0;
    // no epsilon with explicit MPI_UB -- see MPI-standard-2.2 sect 4.1.6
    // explicit MPI_UB and MPI_LB are deleted and newly set -- see MPI-standard-2.2 sect 4.1.7
}

std::vector<struct posInfo> FullResizedTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    ret.push_back(posInfo(pos, add));
    return ret;
}

//=============================
// passAcross
//=============================
bool FullResizedTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        lb,
        extent,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}


//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullSubarrayTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Destructor
//=============================
FullSubarrayTypeInfo::~FullSubarrayTypeInfo(void)
{
    if (arrayOfSizes)
        delete [] arrayOfSizes;
    if (arrayOfSubsizes)
        delete [] arrayOfSubsizes;
    if (arrayOfStarts)
        delete [] arrayOfStarts;
}

//=============================
// Constructor
//=============================
FullSubarrayTypeInfo::FullSubarrayTypeInfo(void)
 : ndims (0),
   arrayOfSizes (NULL),
   arrayOfSubsizes (NULL),
   arrayOfStarts (NULL),
   order (0),
   myPassAcrossFunc (NULL)
{

}

//=============================
// Constructor
//=============================
FullSubarrayTypeInfo::FullSubarrayTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int ndims,
        const int* sizes,
        const int* subsizes,
        const int* starts,
        int order,
        std::vector<Datatype *> oldInfos,
        passDatatypeSubarrayAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        ndims(ndims),
        order(order),
        myPassAcrossFunc (passAcrossFunc)
{

    arrayOfSizes = new int[ndims] ();
    arrayOfSubsizes = new int[ndims] ();
    arrayOfStarts = new int[ndims] ();
    memcpy(arrayOfSizes, sizes, ndims*sizeof(int));
    memcpy(arrayOfSubsizes, subsizes, ndims*sizeof(int));
    memcpy(arrayOfStarts, starts, ndims*sizeof(int));

    // Calculate Sizes
    lb = oldInfos[0]->getLb();
    size = oldInfos[0]->getSize();
    extent = oldInfos[0]->getExtent();
    true_lb = 0;
    true_extent = 0;
    int i=0, inc=1, end=ndims;
    if(!track->getBCoMod()->isOrderC(order))
        i=ndims-1, inc=-1, end=1;
    for (; inc*i < end; i+=inc)
    {
        size*=arrayOfSubsizes[i];
        extent*=arrayOfSizes[i];
        true_lb *= arrayOfSizes[i];
        true_lb += arrayOfStarts[i];
        true_extent *= arrayOfSizes[i];
        true_extent += arrayOfSubsizes[i] - 1;
    }
    true_extent += 1;
    true_lb *= oldInfos[0]->getExtent();
    true_extent *= oldInfos[0]->getExtent();
    myHasExplicitLb = true;
    myHasExplicitUb = true;
    epsilon = 0;

    // no epsilon with explicit MPI_UB -- see MPI-standard-2.2 sect 4.1.6
    // explicit MPI_UB and MPI_LB are newly set to begin and end of array -- see MPI-standard-2.2 sect 4.1.3
}


//=============================
// Copy-Constructor
//=============================
FullSubarrayTypeInfo::FullSubarrayTypeInfo(FullSubarrayTypeInfo& info)
{
    arrayOfSizes = new int[info.ndims] ();
    arrayOfSubsizes = new int[info.ndims] ();
    arrayOfStarts = new int[info.ndims] ();
    memcpy(arrayOfSizes, info.arrayOfSizes, info.ndims*sizeof(int));
    memcpy(arrayOfSubsizes, info.arrayOfSubsizes, info.ndims*sizeof(int));
    memcpy(arrayOfStarts, info.arrayOfStarts, info.ndims*sizeof(int));
    ndims=info.ndims;
    order=info.order;
    myPassAcrossFunc = info.myPassAcrossFunc;
}


//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullSubarrayTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    MustTypemapType parentTypemap = parentInfos[0]->getTypemap(err);
    if (*err != 0)
        return retval;
    MustTypemapType::iterator iterType;
    for (SubarrayWalk mySaW = SubarrayWalk(arrayOfSizes, arrayOfSubsizes, arrayOfStarts, ndims, track->getBCoMod()->isOrderC(order)); (int)mySaW != mySaW.end(); mySaW++)
    {
        for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
        {
            retval.push_back(std::make_pair(iterType->first, iterType->second + (int)mySaW * parentInfos[0]->getExtent()));
        }
    }
    return retval;
}

//=============================
// getRealBlockInfo
//=============================
void FullSubarrayTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    const BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    myBlockInfo = parentBlockInfo;
    MustAddressType pSize = parentInfos[0]->getSize();
    MustAddressType pExtent = parentInfos[0]->getExtent();
    if(parentBlockInfo.overlapped)
    {
        myBlockInfo.overlapped=parentBlockInfo.overlapped;
        myBlockInfo.posA = parentBlockInfo.posA;
        myBlockInfo.posB = parentBlockInfo.posB;
    }
    int i=0, inc=1, end=ndims;
    if(track->getBCoMod()->isOrderC(order))
        i=ndims-1, inc=-1, end=1;
    for (; inc*i < end; i+=inc)
    {
        myBlockInfo = buildStridedBlocklist(myBlockInfo, pExtent, pSize, arrayOfStarts[i] * pExtent, 0, arrayOfSubsizes[i], 0, 1);
        pSize *= arrayOfSubsizes[i];
        pExtent *= arrayOfSizes[i];
    }
    return;
}


//=============================
// getRealBlockList
//=============================
// void FullSubarrayTypeInfo::getRealBlockList(bool &overlap, int* err)
// {
//     myBlockInfo.blocklist.clear();
//     MustBlocklistType parentBlocklist = parentInfos[0]->getBlockList(overlap, err);
//     if (*err != 0)
//         return;
//     MustBlocklistType::iterator iterType;
//     MustAddressType pos=0;
//     MustAddressType pSize = parentInfos[0]->getSize();
//     MustAddressType pExtent = parentInfos[0]->getExtent();
//     for (SubarrayWalk mySaW = SubarrayWalk(arrayOfSizes, arrayOfSubsizes, arrayOfStarts, ndims, track->getBCoMod()->isOrderC(order)); (int)mySaW != mySaW.end(); mySaW++)
//     {
//         for (iterType = parentBlocklist.begin(); iterType != parentBlocklist.end(); iterType++)
//         {
//             myBlockInfo.blocklist.insert(BlocklistEntry(iterType->first + (int)mySaW * pExtent, iterType->second  + (int)mySaW * pExtent, pos + iterType->pos));
//         }
//         pos += pSize;
//     }
//     return;
// }


std::vector<struct posInfo> FullSubarrayTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    MustAddressType parentsize = parentInfos[0]->getSize();
    MustAddressType parentextent = parentInfos[0]->getExtent();
    int nthElem = errorpos / parentsize;
    int elements = size/parentsize;
    errorpos -= nthElem * parentsize;
    pos += nthElem * parentsize;
    for (int i=ndims-1; i>=0; i--)
    {
        elements /= arrayOfSubsizes[i];
        ret.push_back(posInfo(nthElem, arrayOfSubsizes[i], getAddressVector(pos, nthElem, parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthElem, parentextent, MAXDOTSIBLINGS), "subsize"));
        nthElem /= arrayOfSubsizes[i];
    }
    std::reverse(ret.begin(), ret.end());
    return ret;
}


//=============================
// passAcross
//=============================
bool FullSubarrayTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        ndims,
        arrayOfSizes,
        arrayOfSubsizes,
        arrayOfStarts,
        order,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}


//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullDarrayTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Destructor
//=============================
FullDarrayTypeInfo::~FullDarrayTypeInfo(void)
{
    if (arrayOfGsizes)
        delete [] arrayOfGsizes;
    if (arrayOfDistribs)
        delete [] arrayOfDistribs;
    if (arrayOfDargs)
        delete [] arrayOfDargs;
    if (arrayOfPsizes)
        delete [] arrayOfPsizes;
}

//=============================
// Constructor
//=============================
FullDarrayTypeInfo::FullDarrayTypeInfo(void)
 : commSize (0),
   rank (0),
   ndims (0),
   arrayOfGsizes (NULL),
   arrayOfDistribs (NULL),
   arrayOfDargs (NULL),
   arrayOfPsizes (NULL),
   order (0),
   myPassAcrossFunc (NULL)
{

}

//=============================
// Constructor
//=============================
FullDarrayTypeInfo::FullDarrayTypeInfo(
        DatatypeTrack *track,
        MustParallelId pId,
        MustLocationId lId,
        int commSize,
        int rank,
        int ndims,
        const int* gsizes,
        const int* distribs,
        const int* dargs,
        const int* psizes,
        int order,
        std::vector<Datatype *> oldInfos,
        passDatatypeDarrayAcrossP passAcrossFunc
) :
        Datatype(pId, lId, oldInfos, track),
        commSize(commSize),
        rank(rank),
        ndims(ndims),
        order(order),
        myPassAcrossFunc (passAcrossFunc)
{

    arrayOfGsizes = new int[ndims] ();
    arrayOfDistribs = new int[ndims] ();
    arrayOfDargs = new int[ndims] ();
    arrayOfPsizes = new int[ndims] ();
    memcpy(arrayOfGsizes, gsizes, ndims*sizeof(int));
    memcpy(arrayOfDistribs, distribs, ndims*sizeof(int));
    memcpy(arrayOfDargs, dargs, ndims*sizeof(int));
    memcpy(arrayOfPsizes, psizes, ndims*sizeof(int));

    // Calculate Sizes
    int i;
    lb = oldInfos[0]->getLb();
    size = oldInfos[0]->getSize();
    extent = oldInfos[0]->getExtent();
    for (i=0; i<ndims; i++)
    {
        size*=arrayOfGsizes[i];
        extent*=arrayOfGsizes[i];
    }
    size/=commSize;
    myHasExplicitLb = true;
    myHasExplicitUb = true;
    epsilon = 0;

    // TODO: calculate true values
    true_lb = 0;
    true_extent = 0;
    // no epsilon with explicit MPI_UB -- see MPI-standard-2.2 sect 4.1.6
    // explicit MPI_UB and MPI_LB are newly set to begin and end of array -- see MPI-standard-2.2 sect 4.1.4
}

//=============================
// Copy-Constructor
//=============================
FullDarrayTypeInfo::FullDarrayTypeInfo(FullDarrayTypeInfo& info)
{
    arrayOfGsizes = new int[info.ndims] ();
    arrayOfDistribs = new int[info.ndims] ();
    arrayOfDargs = new int[info.ndims] ();
    arrayOfPsizes = new int[info.ndims] ();
    memcpy(arrayOfGsizes, info.arrayOfGsizes, info.ndims*sizeof(int));
    memcpy(arrayOfDistribs, info.arrayOfDistribs, info.ndims*sizeof(int));
    memcpy(arrayOfDargs, info.arrayOfDargs, info.ndims*sizeof(int));
    memcpy(arrayOfPsizes, info.arrayOfPsizes, info.ndims*sizeof(int));
    size=info.size;
    rank=info.rank;
    ndims=info.ndims;
    order=info.order;
    myPassAcrossFunc = info.myPassAcrossFunc;
}


//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullDarrayTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    MustTypemapType parentTypemap = parentInfos[0]->getTypemap(err);
    if (*err != 0)
        return retval;
    MustTypemapType::iterator iterType;
    int* arrayOfSubsizes = new int[ndims] ();
    for (int i = 0; i < ndims; i++)
    {
        arrayOfSubsizes[i] = arrayOfGsizes[i]/arrayOfPsizes[i];
    }
    for (DarrayWalk myDaW = DarrayWalk(arrayOfGsizes, arrayOfDargs, arrayOfDistribs, arrayOfPsizes, arrayOfSubsizes, ndims, rank, track->getBCoMod()->isOrderC(order), track->getBCoMod()); (int)myDaW != myDaW.end(); myDaW++)
    {
        for (iterType = parentTypemap.begin(); iterType != parentTypemap.end(); iterType++)
        {
            retval.push_back(std::make_pair(iterType->first, iterType->second + (int)myDaW * parentInfos[0]->getExtent()));
        }
    }
    delete [] arrayOfSubsizes;
    return retval;
}

//=============================
// getRealBlockInfo
//=============================
void FullDarrayTypeInfo::getRealBlockInfo()
{
    myBlockInfo.clear();
    myBlockInfo.overlapped=false;
    const BlockInfo& parentBlockInfo = parentInfos[0]->getBlockInfo();
    myBlockInfo = parentBlockInfo;
    MustAddressType pSize = parentInfos[0]->getSize();
    MustAddressType pExtent = parentInfos[0]->getExtent();
    if(parentBlockInfo.overlapped)
    {
        myBlockInfo.overlapped=parentBlockInfo.overlapped;
        myBlockInfo.posA = parentBlockInfo.posA;
        myBlockInfo.posB = parentBlockInfo.posB;
    }
    std::vector<int> arrayOfSubsizes = std::vector<int>(ndims);
    for (int i = 0; i < ndims; i++)
    {
        arrayOfSubsizes[i] = arrayOfGsizes[i]/arrayOfPsizes[i];
    }
    int i=0, inc=1, end=ndims;
    std::vector<int> a_starts = FlexCounter(arrayOfPsizes, ndims).tick(rank);
    if(track->getBCoMod()->isOrderC(order))
        i=ndims-1, inc=-1, end=1;
    for (; inc*i < end; i+=inc)
    {
        if (track->getBCoMod()->isDistributeCyclic(arrayOfDistribs[i]))
        {
            if (track->getBCoMod()->isDistributeDfltDarg(arrayOfDargs[i]) || arrayOfDargs[i] == 1)
            {
                myBlockInfo = buildStridedBlocklist(myBlockInfo, pExtent, pSize, a_starts[i] * pExtent, 0, 1, arrayOfPsizes[i] * pExtent, arrayOfSubsizes[i]);
            }
            else
            {
                myBlockInfo = buildStridedBlocklist(myBlockInfo, pExtent, pSize, a_starts[i] * arrayOfDargs[i] * pExtent, 0, arrayOfDargs[i], arrayOfGsizes[i] / arrayOfDargs[i] * pExtent, arrayOfSubsizes[i] / arrayOfDargs[i]);
            }
        }
        else if (track->getBCoMod()->isDistributeBlock(arrayOfDistribs[i]))
        {
            myBlockInfo = buildStridedBlocklist(myBlockInfo, pExtent, pSize, a_starts[i] * arrayOfSubsizes[i] * pExtent, 0, arrayOfSubsizes[i], 0, 1);
        }
        else // myBCoMod->isDistributeNone(arrayOfDistribs[i])
        {
            myBlockInfo = buildStridedBlocklist(myBlockInfo, pExtent, pSize, 0, 0, arrayOfSubsizes[i], 0, 1);
        }
        pSize *= arrayOfSubsizes[i];
        pExtent *= arrayOfGsizes[i];
    }
    return;
}


//=============================
// getRealBlockList
//=============================
// void FullDarrayTypeInfo::getRealBlockList(bool &overlap, int* err)
// {
//     myBlockInfo.blocklist.clear();
//     MustBlocklistType parentBlocklist = parentInfos[0]->getBlockList(overlap, err);
//     if (*err != 0)
//         return;
//     MustBlocklistType::iterator iterType;
//     MustAddressType pos=0;
//     MustAddressType pSize = parentInfos[0]->getSize();
//     MustAddressType pExtent = parentInfos[0]->getExtent();
//     int* arrayOfSubsizes = new int[ndims] ();
//     for (int i = 0; i < ndims; i++)
//     {
//         arrayOfSubsizes[i] = arrayOfGsizes[i]/arrayOfPsizes[i];
//     }
//     for (DarrayWalk myDaW = DarrayWalk(arrayOfGsizes, arrayOfDargs, arrayOfDistribs, arrayOfPsizes, arrayOfSubsizes, ndims, rank, track->getBCoMod()->isOrderC(order), track->getBCoMod()); (int)myDaW != myDaW.end(); myDaW++)
//     {
//         for (iterType = parentBlocklist.begin(); iterType != parentBlocklist.end(); iterType++)
//         {
//             myBlockInfo.blocklist.insert(BlocklistEntry(iterType->first + (int)myDaW * pExtent, iterType->second  + (int)myDaW * pExtent, pos + iterType->pos));
//         }
//         pos += pSize;
//     }
//     delete [] arrayOfSubsizes;
//     return;
// }


std::vector<struct posInfo> FullDarrayTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    std::vector<struct posInfo> ret;
    MustAddressType parentsize = parentInfos[0]->getSize();
    MustAddressType parentextent = parentInfos[0]->getExtent();
    int nthElem = errorpos / parentsize;
    int elements = size/parentsize;
    errorpos -= nthElem * parentsize;
    pos += nthElem * parentsize;
    for (int i=ndims-1; i>=0; i--)
    {
        elements /= (arrayOfGsizes[i]/arrayOfPsizes[i]);
        ret.push_back(posInfo(nthElem, (arrayOfGsizes[i]/arrayOfPsizes[i]), getAddressVector(pos, nthElem, parentsize, MAXDOTSIBLINGS), getAddressVector(add, nthElem, parentextent, MAXDOTSIBLINGS), "subsize"));
        nthElem /= (arrayOfGsizes[i]/arrayOfPsizes[i]);
    }
    std::reverse(ret.begin(), ret.end());
    return ret;
}

//=============================
// passAcross
//=============================
bool FullDarrayTypeInfo::passAcross (
        int givenRank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    MustRemoteIdType baseTypeRemoteId = 0;
    if (parentInfos[0])
        baseTypeRemoteId = parentInfos[0]->getRemoteId();

    (*myPassAcrossFunc) (
        givenRank,
        hasHandle,
        handle,
        this->getRemoteId(),
        creationPId,
        creationLId,
        (int) myIsCommited,
        commitPId,
        commitLId,
        //
        commSize,
        rank,
        ndims,
        arrayOfGsizes,
        arrayOfDistribs,
        arrayOfDargs,
        arrayOfPsizes,
        order,
        baseTypeRemoteId,
        //
        toPlaceId
        );

    return true;
}

//=============================
// getRealTypesig
//=============================
void FullContiguousTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0)
        return;
    int myCount = count;
    MustTypesig parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
     MustTypesig::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullContiguousTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0)
        return;
    int myCount = count;
    MustTypesigType parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
     MustTypesigType::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullVectorTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0 || blocklength == 0)
        return;
    int myCount = count * blocklength;
    MustTypesig parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesig::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullVectorTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0 || blocklength == 0)
        return;
    int myCount = count * blocklength;
    MustTypesigType parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesigType::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullHVectorTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0 || blocklength == 0)
        return;
    int myCount = count * blocklength;
    MustTypesig parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesig::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullHVectorTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0 || blocklength == 0)
        return;
    int myCount = count * blocklength;
    MustTypesigType parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesigType::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullIndexedTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0)
        return;
    int myCount =0;
    for (int i=0; i<count; i++)
        myCount += arrayOfBlocklengths[i];
    MustTypesig parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesig::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullIndexedTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0)
        return;
    int myCount =0;
    for (int i=0; i<count; i++)
        myCount += arrayOfBlocklengths[i];
    MustTypesigType parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesigType::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullHIndexedTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0)
        return;
    int myCount =0;
    for (int i=0; i<count; i++)
        myCount += arrayOfBlocklengths[i];
    MustTypesig parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesig::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullHIndexedTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0)
        return;
    int myCount =0;
    for (int i=0; i<count; i++)
        myCount += arrayOfBlocklengths[i];
    MustTypesigType parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesigType::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullStructTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0)
        return;
    MustTypesig parentTypesig;
    for (int i=count-1; i>=0; i--){
        if (arrayOfBlocklengths[i] == 0)
        { // blocklength == 0, so next entry
            continue;
        }
        parentTypesig = parentInfos[i]->getTypesig(typesig, err);
        if (parentTypesig.empty())
        { // typesig empty, so next
            continue;
        }
        if (parentTypesig.size() == 1)
        { // just one basetype in parent
            if (!typesig->empty() && typesig->front().second == parentTypesig.front().second)
            { // my typesig is not empty and first basetype is equal to parents basetype
                typesig->front().first += arrayOfBlocklengths[i] * parentTypesig.front().first;
            }
            else
            { // new basetype, so create new entry
                typesig->push_front(std::make_pair(arrayOfBlocklengths[i] * parentTypesig.front().first, parentTypesig.front().second));
            }
            continue;
        }
        // if we are here, we have and heterogenous parent typesig
        MustTypesig::iterator insertIter = typesig->begin();
        MustTypesig::iterator backIter = parentTypesig.end();
        if (!typesig->empty() && typesig->front().second == parentTypesig.back().second)
        {   // first basetype of my typesig and last basetype of parent are equal
            // so condense them
            typesig->front().first += parentTypesig.back().first;
            backIter--;
        }
        typesig->insert(insertIter, parentTypesig.begin(), backIter);
        if (parentTypesig.front().second == parentTypesig.back().second)
        {   // first and last basetype of parent are equal
            // so condense them
            parentTypesig.back().first += parentTypesig.front().first;
            parentTypesig.pop_front();
            insertIter++;
        }
        for (int i=1; i<arrayOfBlocklengths[i]; i++)
            typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    }
    return;
}

//=============================
// getRealTypesig
//=============================
void FullStructTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    if (count == 0)
        return;
    MustTypesigType parentTypesig;
    for (int i=count-1; i>=0; i--){
        if (arrayOfBlocklengths[i] == 0)
        { // blocklength == 0, so next entry
            continue;
        }
        parentTypesig = parentInfos[i]->getTypesig(typesig, err);
        if (parentTypesig.empty())
        { // typesig empty, so next
            continue;
        }
        if (parentTypesig.size() == 1)
        { // just one basetype in parent
            if (!typesig->empty() && typesig->front().second == parentTypesig.front().second)
            { // my typesig is not empty and first basetype is equal to parents basetype
                parentTypesig.front().first += arrayOfBlocklengths[i] * parentTypesig.front().first;
            }
            else
            { // new basetype, so create new entry
                typesig->push_front(std::make_pair(arrayOfBlocklengths[i] * parentTypesig.front().first, parentTypesig.front().second));
            }
            continue;
        }
        // if we are here, we have and heterogenous parent typesig
        MustTypesigType::iterator insertIter = typesig->begin();
        MustTypesigType::iterator backIter = parentTypesig.end();
        if (!typesig->empty() && typesig->front().second == parentTypesig.back().second)
        {   // first basetype of my typesig and last basetype of parent are equal
            // so condense them
            typesig->front().first += parentTypesig.back().first;
            backIter--;
        }
        typesig->insert(insertIter, parentTypesig.begin(), backIter);
        if (parentTypesig.front().second == parentTypesig.back().second)
        {   // first and last basetype of parent are equal
            // so condense them
            parentTypesig.back().first += parentTypesig.front().first;
            parentTypesig.pop_front();
            insertIter++;
        }
        for (int i=1; i<arrayOfBlocklengths[i]; i++)
            typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    }
    return;
}

//=============================
// getRealTypesig
//=============================
void FullIndexedBlockTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    int myCount = count * blocklength;
    if (myCount == 0)
        return;
    MustTypesig parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesig::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullIndexedBlockTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    int myCount = count * blocklength;
    if (myCount == 0)
        return;
    MustTypesigType parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesigType::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullResizedTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    *typesig = parentInfos[0]->getTypesig(typesig, err);
}

//=============================
// getRealTypesig
//=============================
void FullResizedTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    *typesig = parentInfos[0]->getTypesig(typesig, err);
}

//=============================
// getRealTypesig
//=============================
void FullSubarrayTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    int myCount = 1;
    for (int i = 0; i < ndims; i++)
    {
        myCount*=arrayOfSubsizes[i];
    }
    MustTypesig parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesig::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullSubarrayTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    int myCount = 1;
    for (int i = 0; i < ndims; i++)
    {
        myCount*=arrayOfSubsizes[i];
    }
    MustTypesigType parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesigType::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullDarrayTypeInfo::getRealTypesig(MustTypesig* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    int myCount = 1;
    for (int i = 0; i<ndims; i++)
    {
        myCount*=arrayOfGsizes[i];
    }
    myCount/=commSize;
    MustTypesig parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesig::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}

//=============================
// getRealTypesig
//=============================
void FullDarrayTypeInfo::getRealTypesig(MustTypesigType* typesig, int* err)
{
    *err=1;
    if (typesig == NULL)
        return;
    typesig->clear();
    *err=0;
    int myCount = 1;
    for (int i = 0; i<ndims; i++)
    {
        myCount*=arrayOfGsizes[i];
    }
    myCount/=commSize;
    MustTypesigType parentTypesig = parentInfos[0]->getTypesig(typesig, err);
    if (parentTypesig.empty())
    {
        return;
    }
    if (parentTypesig.size() == 1)
    {
        typesig->push_back(std::make_pair(myCount * parentTypesig.front().first, parentTypesig.front().second));
        return;
    }
    MustTypesigType::iterator insertIter = typesig->begin();
    typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    if (parentTypesig.front().second == parentTypesig.back().second)
    {   // first and last basetype of parent are equal
        // so condense them
        parentTypesig.back().first += parentTypesig.front().first;
        parentTypesig.pop_front();
        insertIter++;
    }
    for (int i=1; i<myCount; i++)
        typesig->insert(insertIter, parentTypesig.begin(), parentTypesig.end());
    return;
}
/*EOF*/
