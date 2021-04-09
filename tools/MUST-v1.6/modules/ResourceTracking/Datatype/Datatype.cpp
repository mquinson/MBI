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
 * @file Datatype.cpp
 *       @see MUST::Datatype.
 *
 *  @date 22.06.2011
 *  @author Joachim Protze
 */

#include "GtiMacros.h"

#include "DatatypeTrackDerivedStorage.h"
#include "DatatypeTrack.h"
#include <sstream>

using namespace must;

std::vector<MustAddressType> must::getAddressVector(MustAddressType base, int offset, MustAddressType stride, size_t n){
    return getAddressVector(base, stride * offset, stride, n);
}

std::vector<MustAddressType> must::getAddressVector(MustAddressType base, MustAddressType offset, MustAddressType stride, size_t n)
{
    std::vector<MustAddressType> ret;
    ret.push_back(base);
    for (size_t i=1; i<n; i++)
        ret.push_back(base+offset+i*stride);
    return ret;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// Datatype
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Destructor
//=============================
Datatype::~Datatype(void)
{
    //Nothing to do
    for (std::vector<Datatype *>::size_type i = 0; i < parentInfos.size(); i++)
        parentInfos[i]->erase();
    parentInfos.clear();
}

//=============================
// Constructor
//=============================
Datatype::Datatype (
        MustParallelId creationPId,
        MustLocationId creationLId,
        std::vector<Datatype *> oldInfos,
        DatatypeTrack * track
) :
        HandleInfoBase ("Datatype"),
        track(track),
        myTypesig(),
        cachedTypesig(false),
        cachedLongTypesig(false),
        myBlockInfo(),
        cachedBlocklist(false),
        myTypemap(),
        cachedTypemap(false),
        maxNoSelfoverlap(0),
        minSelfoverlap(((int)1 << (sizeof(int)*8-1)) ^ -1),
        myIsNull(false),
        myIsPredefined(false),
        myIsCommited(false),
        myIsC(oldInfos[0]->isC()),
        myIsFortran(oldInfos[0]->isFortran()),
        myHasExplicitLb(oldInfos[0]->hasExplicitLb()),
        myHasExplicitUb(oldInfos[0]->hasExplicitUb()),
        alignment(oldInfos[0]->getAlignment()),
        epsilon(oldInfos[0]->getEpsilon()),
        creationPId(creationPId),
        creationLId(creationLId),
        commitPId(0),
        commitLId(0),
        parentInfos(oldInfos)
{}

Datatype::Datatype ():
        HandleInfoBase ("Datatype"),
        myTypesig(),
        cachedTypesig(false),
        cachedLongTypesig(false),
        myBlockInfo(),
        cachedBlocklist(false),
        myTypemap(),
        cachedTypemap(false),
        maxNoSelfoverlap(0),
        minSelfoverlap(((int)1 << (sizeof(int)*8-1)) ^ -1),
        myIsNull(false),
        myIsPredefined(false),
        myIsCommited(false),
        epsilon(0),
        commitPId(0),
        commitLId(0),
        parentInfos()
{}

// void Datatype::getRealTypesig(MustTypesig * typesig, int * err){}
// void Datatype::getRealTypesig(MustTypesigType * typesig, int * err){}

std::pair<int,int> Datatype::getSelfoverlapCache(void){
    return std::make_pair(maxNoSelfoverlap, minSelfoverlap);
}
void Datatype::setMaxNoOverlap(int nooverlap){
    maxNoSelfoverlap = nooverlap;
}
void Datatype::setMinOverlap(int overlap){
    minSelfoverlap = overlap;
}

//=============================
// getReferencedTypes
//=============================
std::list<I_Datatype*> Datatype::getReferencedTypes()
{
    std::list<I_Datatype*> ret;

    for (std::vector<Datatype *>::size_type i = 0; i < parentInfos.size(); i++)
    {
        assert (parentInfos[i]);

        //A temporary list stored the parent type and its recursive parent types
        std::list<I_Datatype*> temp; //DOES NOT NEEDS TO BE RECUSIVE=> = parentInfos[i]->getReferencedTypes();
        temp.push_back(parentInfos[i]);

        //Only add each type once! So we check against the return list
        std::list<I_Datatype*>::iterator a,b;
        for (a = temp.begin(); a != temp.end(); a++)
        {
            I_Datatype *item = *a;

            for (b = ret.begin(); b != ret.end(); b++)
            {
                if (*b == item)
                    break;
            }

            if (b == ret.end())
                ret.push_back(item);
        }
    }

    return ret;
}

//=============================
// commit
//=============================

void Datatype::commit(MustParallelId pId, MustLocationId lId)
{
    commitPId = pId;
    commitLId = lId;
    myIsCommited = true;
}

//=============================
// getInfo
//=============================

bool Datatype::isNull(void) const
{
    return myIsNull;
}
bool Datatype::isPredefined(void) const
{
    return myIsPredefined;
}
bool Datatype::isOptional(void) const
{
    if (myIsPredefined)
        return ((FullBaseTypeInfo*)this)->isOptional;
    return false;
}
bool Datatype::isForReduction(void) const
{
    if (myIsPredefined)
        return ((FullBaseTypeInfo*)this)->isForReduction;
    return false;
}
bool Datatype::isBoundMarker(void) const
{
    if (myIsPredefined)
        return ((FullBaseTypeInfo*)this)->isBoundMarker;
    return false;
}
bool Datatype::isCommited(void) const
{
    return myIsCommited;
}
bool Datatype::isC(void) const
{
    return myIsC;
}
bool Datatype::isFortran(void) const
{
    return myIsFortran;
}
bool Datatype::hasExplicitLb(void) const
{
    return myHasExplicitLb;
}
bool Datatype::hasExplicitUb(void) const
{
    return myHasExplicitUb;
}

//=============================
// getHistory
//=============================
MustParallelId Datatype::getCreationPId(void) const
{
    return creationPId;
}

MustLocationId Datatype::getCreationLId(void) const
{
    return creationLId;
}

MustParallelId Datatype::getCommitPId(void) const
{
    return commitPId;
}

MustLocationId Datatype::getCommitLId(void) const
{
    return commitLId;
}

//=============================
// getSizes
//=============================

MustAddressType Datatype::getLb(void) const
{
    return lb;
}

MustAddressType Datatype::getUb(void) const
{
    return lb + extent;
}

MustAddressType Datatype::getExtent(void) const
{
    return extent;
}

MustAddressType Datatype::getSize(void) const
{
    return size;
}

//=============================
// getTrueSizes
//=============================

MustAddressType Datatype::getTrueLb(void) const
{
    return true_lb;
}

MustAddressType Datatype::getTrueUb(void) const
{
    return true_lb + true_extent;
}

MustAddressType Datatype::getTrueExtent(void) const
{
    return true_extent;
}

int Datatype::getEpsilon(void) const
{
    return epsilon;
}

int Datatype::getAlignment(void) const
{
    return alignment;
}

std::vector<struct posInfo> Datatype::posToPath(MustAddressType& errorpos)
{
    MustAddressType add=0, pos=0;
    return posToPath(errorpos, add, pos);
}

bool Datatype::printDatatypePos(std::ostream &out, MustAddressType errorpos){
    if (errorpos > size)
        out << "[" << (int)(errorpos/size) << "]";
    return printRealDatatypePos(out, errorpos % size);
}

bool Datatype::printDatatypeLongPos(std::ostream &out, MustAddressType errorpos){
    if (errorpos > size)
        out << "[" << (int)(errorpos/size) << "]";
    return printRealDatatypeLongPos(out, errorpos % size);
}

bool Datatype::printRealDatatypePos(std::ostream &out, MustAddressType errorpos){
    std::vector<struct posInfo> path = posToPath(errorpos);
    std::vector<struct posInfo>::iterator iter=path.begin(), end=path.end();
    for (; iter != end; iter++)
        out << "[" << iter->index << "]";
    if (parentInfos.size()==1)
        return parentInfos[0]->printRealDatatypePos(out, errorpos);
    return parentInfos[path[0].index]->printRealDatatypePos(out, errorpos);
}

bool Datatype::printRealDatatypeLongPos(std::ostream &out, MustAddressType errorpos){
    std::vector<struct posInfo> path = posToPath(errorpos);
    std::vector<struct posInfo>::iterator iter=path.begin(), end=path.end();
    out << "(" << std::uppercase << kindName() << ")";
    for (; iter != end; iter++)
        out << "[" << iter->index << "]";
    if (parentInfos.size()==1)
        return parentInfos[0]->printRealDatatypeLongPos(out, errorpos);
    return parentInfos[path[0].index]->printRealDatatypeLongPos(out, errorpos);
}

bool Datatype::printDatatypeDotOverlap(
    std::ostream &out, 
    MustAddressType errorposA, 
    MustAddressType addressA, 
    std::string callNodeA, 
    I_Datatype * typeB, 
    MustAddressType errorposB, 
    MustAddressType addressB, 
    std::string callNodeB
)
{
    Datatype * leftType, *rightType;
    MustAddressType leftAddress, rightAddress, leftPos, rightPos;
    std::string leftNode, rightNode;
    bool ret;
    DatatypeDotNode* retNode=NULL;
    DatatypeForest f;
    int level;
    MustAddressType tempadd;
    std::string edgeText="";
    
    if(addressA < addressB)
    {
        leftType = this;
        tempadd = addressA;
        leftAddress = 0;
        leftPos = errorposA;
        leftNode = callNodeA;
        rightType = (Datatype*)typeB;
        rightAddress = addressB-addressA;
        rightPos = errorposB;
        rightNode = callNodeB;
    }
    else
    {
        leftType = (Datatype*)typeB;
        tempadd = addressB;
        leftAddress = 0;
        leftPos = errorposB;
        leftNode = callNodeB;
        rightType = this;
        rightAddress = addressA-addressB;
        rightPos = errorposA;
        rightNode = callNodeA;
    }
    {
        std::stringstream ss;
        ss << leftNode << "(buf= 0x"<< std::hex << tempadd <<")";
        leftNode = ss.str();
    }
    tempadd = leftAddress;
    if (leftPos >= leftType->getSize())
    {
        std::stringstream strstream;
        strstream << "[" << (int)(leftPos/leftType->getSize()) << "]";
        edgeText = strstream.str();
        leftAddress += leftType->getExtent() * (int)(leftPos/leftType->getSize());
        leftPos%=leftType->getSize();
    }
    ret = leftType->fillOverlapTree(f, retNode, leftPos, leftAddress, level, 0);
    retNode = f.insertParentNode(level, retNode, leftNode, tempadd, edgeText, 0);
    edgeText="";
    tempadd = rightAddress;
    {
        std::stringstream ss;
        ss << rightNode << "(buf= +0x"<< std::hex << tempadd <<")";
        rightNode = ss.str();
    }
    if (rightPos >= rightType->getSize())
    {
        std::stringstream strstream;
        int nth = rightPos / rightType->getSize();
        strstream << "[" << nth << "]";
        edgeText = strstream.str();
        rightAddress += rightType->getExtent() * nth;
        rightPos %= rightType->getSize();
    }
    ret = rightType->fillOverlapTree(f, retNode, rightPos, rightAddress, level, 1);
    retNode = f.insertParentNode(level, retNode, rightNode, tempadd, edgeText, 1);

    f.toString(out);

    return true;
}


bool Datatype::printDatatypeDotTypemismatch(
    std::ostream &out, 
    MustAddressType errorpos, 
    std::string callNodeA, 
    I_Datatype * typeB, 
    std::string callNodeB
)
{
    Datatype * leftType, *rightType;
    MustAddressType leftPos, rightPos;
    std::string leftNode, rightNode;
    bool ret;
    DatatypeDotNode* retNode=NULL;
    DatatypeForest f;
    int level;
    MustAddressType tempadd=0;
    std::string edgeText="";
    
    leftType = this;
    leftPos = errorpos;
    leftNode = callNodeA;
    rightType = (Datatype*)typeB;
    rightPos = errorpos;
    rightNode = callNodeB;

    if (leftPos >= leftType->getSize())
    {
        std::stringstream strstream;
        strstream << "[" << (int)(leftPos/leftType->getSize()) << "]";
        edgeText = strstream.str();
        tempadd = (int)(leftPos/leftType->getSize()) * leftType->getSize();
        leftPos%=leftType->getSize();
    }
    ret = leftType->fillTypemismatchTree(f, retNode, leftPos, tempadd, level, 0);
    retNode = f.insertParentNode(level, retNode, leftNode, 0, edgeText, 0);
    edgeText="";
    if (rightPos >= rightType->getSize())
    {
        std::stringstream strstream;
        int nth = rightPos / rightType->getSize();
        strstream << "[" << nth << "]";
        edgeText = strstream.str();
        tempadd = (int)(rightPos/rightType->getSize()) * rightType->getSize();
        rightPos %= rightType->getSize();
    }
    tempadd=0;
    ret = rightType->fillTypemismatchTree(f, retNode, rightPos, tempadd, level, 1);
    retNode = f.insertParentNode(level, retNode, rightNode, 1, edgeText, 1);

    f.toString(out);

    return true;
}


bool Datatype::fillOverlapTree(
    DatatypeForest& f, 
    DatatypeDotNode*& retNode, 
    MustAddressType errorpos, 
    MustAddressType address, 
    int& level,
    int type
)
{
    MustAddressType pos;
    std::vector<struct posInfo> infos = posToPath(errorpos, pos,address);
    bool ret;
    if (parentInfos.size()==1)
        ret = parentInfos[0]->fillOverlapTree(f, retNode, errorpos, address, level, type);
    else
        ret = parentInfos[infos[0].index]->fillOverlapTree(f, retNode, errorpos, address, level, type);
    std::vector<struct posInfo>::reverse_iterator infoIter;
    std::string nodeText, edgeText;
    for (infoIter = infos.rbegin(); infoIter!=infos.rend(); infoIter++)
    {
        std::stringstream nstream, estream;
        if (&(*infoIter)==&(infos[0]))
            nstream << "MPI_Type_" << kindName();
        nstream << "(" << infoIter->name << "=" << infoIter->count << ")";
        nodeText = nstream.str();
        estream << "[" << infoIter->index << "]";
        edgeText = estream.str();
        retNode = f.insertParentNode(level, retNode, nodeText, infoIter->add[0], edgeText, type);
        nodeText="";
        for (int i=infoIter->index+1, j=1; i<infoIter->count && j<MAXDOTSIBLINGS;i++, j++)
        {
            std::stringstream tmpstream;
            tmpstream << "[" << i << "]";
            edgeText = tmpstream.str();
            f.insertChildNode(level-1, retNode, nodeText, infoIter->add[j], edgeText, type);
        }
        level++;
    }
    return ret;
}

bool Datatype::fillUpLeafs(
    DatatypeForest& f, 
    DatatypeDotNode* pNode,
    MustAddressType errorpos, 
    MustAddressType pos,
    int& leafs,
    int type
)
{
    MustAddressType add;
    std::vector<struct posInfo> infos = posToPath(errorpos, pos, add);
    struct posInfo* info = &(infos.back());
    std::string nodeText, edgeText;
    {
        std::stringstream nstream, estream;
        nstream << "(" << info->name << "=" << info->count << ")";
        nodeText = nstream.str();
        estream << "[" << infos[0].index << "]";
        edgeText = estream.str();
    }
    pNode = f.insertChildNode(1, pNode, nodeText, info->pos[0], edgeText, type);
    if (parentInfos.size()==1)
        nodeText = ((FullBaseTypeInfo*)parentInfos[0])->predefName;
    else
        nodeText = ((FullBaseTypeInfo*)parentInfos[infos[0].index])->predefName;
    for (int i=0; i<info->count && leafs>0;i++, leafs--)
    {
        std::stringstream tmpstream;
        tmpstream << "[" << i << "]";
        edgeText = tmpstream.str();
        f.insertChildNode(0, pNode, nodeText, info->pos[i], edgeText, type);
    }
    return true;
}

bool Datatype::fillTypemismatchTree(
    DatatypeForest& f, 
    DatatypeDotNode*& retNode, 
    MustAddressType errorpos, 
    MustAddressType pos,
    int& level,
    int type
)
{
    MustAddressType add=0;
    int leafs = MAXDOTSIBLINGS;
    std::vector<struct posInfo> infos = posToPath(errorpos, pos, add);
    bool ret;
    if (parentInfos.size()==1)
        ret = parentInfos[0]->fillTypemismatchTree(f, retNode, errorpos, pos, level, type);
    else
        ret = parentInfos[infos[0].index]->fillTypemismatchTree(f, retNode, errorpos, pos, level, type);
    std::vector<struct posInfo>::reverse_iterator infoIter;
    std::string nodeText, edgeText;
    for (infoIter = infos.rbegin(); infoIter!=infos.rend(); infoIter++)
    {
        std::stringstream nstream, estream;
        if (&(*infoIter)==&(infos[0]))
            nstream << "MPI_Type_" << kindName();
        nstream << "(" << infoIter->name << "=" << infoIter->count << ")";
        nodeText = nstream.str();
        estream << "[" << infoIter->index << "]";
        edgeText = estream.str();
        retNode = f.insertParentNode(level, retNode, nodeText, infoIter->pos[0], edgeText, type);
        nodeText="";
        for (int i=infoIter->index+1, j=1; i<infoIter->count && j<MAXDOTSIBLINGS;i++, j++)
        {
            std::stringstream tmpstream;
            tmpstream << "[" << i << "]";
            edgeText = tmpstream.str();
            if(level==2 && leafs > 0)
            {
                fillUpLeafs(f, retNode, infoIter->pos[j]-infos[0].pos[0], infos[0].pos[0], leafs, type);
            }
            else
                f.insertChildNode(level-1, retNode, nodeText, infoIter->pos[j], edgeText, type);
            if(level==1)
                leafs--;
        }
        level++;
    }
    return ret;
}

//=============================
// printInfo
//=============================
bool Datatype::printInfo (
        std::stringstream &out,
        std::list<std::pair<MustParallelId,MustLocationId> > *pReferences)
{
    //NULL
    if (myIsNull)
    {
        out << "MPI_DATATYPE_NULL";
        return true;
    }

    //Predefined
    if (myIsPredefined)
    {
        out << getPredefinedName ();
        if (isBoundMarker())
            out << " is a bound marker";
        if (isForReduction())
            out << " is for minloc/maxloc operations";
        return true;
    }

    //User Handle
#ifdef MUST_DEBUG
    out << "MUST_DT_INFO: lb: " << lb << " extent: " << extent << " ub: " << lb+extent << " size: " << size << std::endl;
    out << "MUST_DT_INFO: true_lb: " << true_lb << " true_extent: " << true_extent << " true_ub: " << true_lb + true_extent << std::endl << std::endl;
#endif
    //blocklist
/*    bool overlapped;
    MustBlocklistType blocklist = getBlockList(overlapped);
    MustBlocklistType::iterator iter;
    std::cout << "Blocklist = {";
    for(iter = blocklist.begin(); iter !=blocklist.end(); iter++){
        std::cout << "( " << iter->first << ", " << iter->second << "), ";
    }
    std::cout << "}" << std::endl;*/

    //A user defined datatype
    pReferences->push_back(std::make_pair (creationPId, creationLId));
    out << "Datatype created at reference " << pReferences->size();

    //Category stuff
    if (myIsC)
        out << " is for C";
    if (myIsFortran)
        out << " is for Fortran";

    //Is commited?
    if (myIsCommited)
    {
        pReferences->push_back(std::make_pair (commitPId, commitLId));
        out << ", commited at reference " << pReferences->size();
    }
    //Base types
//    std::list<MustDatatypeType> refTypes = getReferencedTypes();
    if (!parentInfos.empty())
    {
        out << ", based on the following type(s): {";
        std::vector<Datatype *>::iterator refIter;
        for (refIter = parentInfos.begin(); refIter != parentInfos.end(); refIter++)
        {
           Datatype* dt2 = *refIter;

            if (dt2 == NULL)
                continue;

            if (refIter != parentInfos.begin())
                out << ",";

            if (dt2->isPredefined())
            {
                out << " " << dt2->getPredefinedName();
            }
            else
            {
                pReferences->push_back(std::make_pair (dt2->getCreationPId(), dt2->getCreationLId()));
                out << " type created at reference " << pReferences->size();
            }
        }
    }
    out << "}";
//    printTypemapString(out);

    return true;
}

//=============================
// getPredefinedInfo
//=============================
MustMpiDatatypePredefined Datatype::getPredefinedInfo ()
{
    if (!myIsPredefined)
        return MUST_MPI_DATATYPE_UNKNOWN;

    return ((FullBaseTypeInfo*)this)->predefValue;
}

//=============================
// getPredefinedName
//=============================
std::string Datatype::getPredefinedName (void)
{
    return "";//Predefined datatypes overwrite this function.
}


//=============================
// handleIterInc
// return true: reached end of repetitions!
//=============================
inline bool handleIterInc (
        const MustTypesig& typesig,
        MustTypesig::const_iterator& iter,
        int& i,
        int count
        )
{
    iter++;
    if (iter==typesig.end())
    { // reached end of type
        iter = typesig.begin();
        i++;
        if (i >= count)
        { // reached end of repetitions
            return true;
        }
    }
    return false;
}

//=============================
// handleMpiByte
// return true: -> all mached successfully!
//              -> valid iterators =! .end() guaranteed
//=============================
MustMessageIdNames Datatype::handleMpiByte (
        const MustTypesig& typesigA,
        MustTypesig::const_iterator& iterA,
        int& iA,
        int countA,
        const MustTypesig& typesigB,
        MustTypesig::const_iterator& iterB,
        int& iB,
        int countB,
        MustAddressType* errorpos
        )
{
    int remainingA, remainingB;
    while (iterA->second->predefValue == MUST_MPI_BYTE || iterB->second->predefValue == MUST_MPI_BYTE)
    {
        if (iterA->second->size * iterA->first == iterB->second->size * iterB->first)
        { // clean match, success
            *errorpos += iterA->second->size * iterA->first;
            if (handleIterInc(typesigA, iterA, iA, countA))
                return MUST_MESSAGE_NO_ERROR;
            if (handleIterInc(typesigB, iterB, iB, countB))
                return MUST_MESSAGE_NO_ERROR; // should never be reached, since type A finishes first!
            continue;
        }
        remainingA = iterA->first;
        remainingB = iterB->first;
        while (remainingA > 0 || remainingB > 0)
        {
            if (iterA->second->predefValue != MUST_MPI_BYTE && iterB->second->predefValue != MUST_MPI_BYTE && iterA->second->predefValue != iterB->second->predefValue)
            {
                return MUST_ERROR_TYPEMATCH_MISSMATCH_BYTE; // type missmatch
            } // else: both types are equal or one is MPI_BYTE
            if (remainingA * iterA->second->size == remainingB * iterB->second->size)
            { // ends of blocks matches
                *errorpos += iterA->second->size * remainingA;
                if (handleIterInc(typesigA, iterA, iA, countA))
                    return MUST_MESSAGE_NO_ERROR; // reached end of repetitions!
                if (handleIterInc(typesigB, iterB, iB, countB))
                    return MUST_MESSAGE_NO_ERROR; // should never be reached, since type A finishes first!
                continue;
            }
            if (remainingA * iterA->second->size < remainingB * iterB->second->size)
            { // type A submits full block, type B remains a residual
                *errorpos += iterA->second->size * remainingA;
                if ((remainingA * iterA->second->size) % iterB->second->size > 0)
                {
                    return MUST_ERROR_TYPEMATCH_MISSMATCH_BYTE; // type missmatch in multiplicy of MPI_BYTE
                }
                remainingB -= remainingA * iterA->second->size / iterB->second->size;
                if (handleIterInc(typesigA, iterA, iA, countA))
                    return MUST_MESSAGE_NO_ERROR; // reached end of repetitions!
                remainingA = iterA->first;
                continue;
            }
            // else: (remainingA * iterA->second->size > remainingB * iterB->second->size)
            // type B submits full block, type A remains a residual
            *errorpos += iterB->second->size * remainingB;
            if ((remainingB * iterB->second->size) % iterA->second->size > 0)
            {
                return MUST_ERROR_TYPEMATCH_MISSMATCH_BYTE; // type missmatch in multiplicy of MPI_BYTE
            }
            remainingA -= remainingB * iterB->second->size / iterA->second->size;
            if (handleIterInc(typesigB, iterB, iB, countB))
                return MUST_MESSAGE_NO_ERROR; // reached end of repetitions!
            remainingB = iterB->first;
            continue;
        }
    }
    return MUST_MESSAGE_NO_ERROR;
}

//=============================
// checkWhetherSubsetOfB
//=============================
MustMessageIdNames Datatype::checkWhetherSubsetOfB (
        int countA,
        I_Datatype* typeB,
        int countB,
        MustAddressType* errorpos
        )
{
    MustTypesig * typesigP=NULL;
    int err = 0, iA = 0, iB = 0;
    Datatype * datatypeB = (Datatype*)typeB;
    const MustTypesig& typesigA = getTypesig(typesigP, &err);
    if (err!=0)
        return MUST_ERROR_TYPEMATCH_INTERNAL_TYPESIG; // error on getting typesig of A
    const MustTypesig& typesigB = datatypeB->getTypesig(typesigP, &err);
    if (err!=0)
        return MUST_ERROR_TYPEMATCH_INTERNAL_TYPESIG; // error on getting typesig of B
    MustTypesig::const_iterator iterA = typesigA.begin();
    MustTypesig::const_iterator iterB = typesigB.begin();

/*    std::cout << "isASubsetOfB" << std::endl;
    printTypemapString(std::cout);
    std::cout << std::endl;
    typeB->printTypemapString(std::cout);
    std::cout << std::endl;*/
    if (typesigA.size()==1 || typesigB.size()==1)
    { // one type consists of just one basetype
        //Is one type empty?
        /**
         *@todo the below two if's where introduced as a bugfix, they need to be reviewed by Joachim
         */
        if (typesigB.size()==0)
            return MUST_MESSAGE_NO_ERROR;
        if (typesigA.size()==0)
            return MUST_ERROR_TYPEMATCH_MISSMATCH;

        if (typesigA.front().second->predefValue != typesigB.front().second->predefValue)
            return MUST_ERROR_TYPEMATCH_MISSMATCH;
        if (typesigA.size()!=1) // type A consists of more than one basetypes
        {
            *errorpos += typesigA.front().second->size * typesigA.front().first;
            return MUST_ERROR_TYPEMATCH_MISSMATCH;
        }
        if (typesigB.size()!=1) // type B consists of more than one basetypes
        {
            *errorpos += typesigB.front().second->size * typesigB.front().first;
            return MUST_ERROR_TYPEMATCH_MISSMATCH;
        }
        return MUST_MESSAGE_NO_ERROR;
    }
    
    // loop the repetitions of type A and B:
    while (iA < countA && iB < countB)
    {
        // loop over type A and B:
        for(;iterA != typesigA.end() && iterB != typesigB.end(); iterA++, iterB++)
        {
            if (iterA->second->predefValue == MUST_MPI_BYTE || iterB->second->predefValue == MUST_MPI_BYTE)
            { // we need some special handling for MPI_BYTE!
                MustMessageIdNames ret = handleMpiByte(typesigA, iterA, iA, countA, typesigB, iterB, iB, countB, errorpos);
                // error, so return it
                if (ret != MUST_MESSAGE_NO_ERROR)
                    return ret;
                // may have reached end of repetitions!
                if (!(iA < countA && iB < countB))
                    return MUST_MESSAGE_NO_ERROR;
            }
            if(iterA->second->predefValue != iterB->second->predefValue)
                return MUST_ERROR_TYPEMATCH_MISSMATCH; // type missmatch
            if(iterA->first != iterB->first)
                break; // length missmatch
            *errorpos += iterA->second->size * iterA->first;
        }

        /////////////////////
        // handle limitation of for-loop: copy of one type is finished

            if (iterA == typesigA.end())
            { // we are at the end of typesigA, next turn
                iA++;
                iterA = typesigA.begin();
                continue; // if both types are finished, for-loop will fail on entry and next if is reached
            }
            if (iterB == typesigB.end())
            { // we are at the end of typesigB, next turn
                iB++;
                iterB = typesigB.begin();
                continue;
            }

        // END handle limitation of for-loop
        /////////////////////

        /////////////////////
        // handle break condition:

            if (&(*iterA) == &(typesigA.back()) && iA + 1 == countA)
            {
                if (typesigA.back().first <= iterB->first || /* last block of type A may be subset of current block of B */
                    /* or it subsets last block and first block of B: */
                    /* backA <= backB + frontB */
                    (&(*iterB) == &(typesigB.back()) && typesigB.back().second == typesigB.front().second && typesigA.back().first < typesigB.back().first + typesigB.front().first))
                {
                    *errorpos += iterA->second->size * iterA->first;
                    return MUST_MESSAGE_NO_ERROR; // match for last block of A
                }
                return MUST_ERROR_TYPEMATCH_MISSMATCH; // no match for last block of A
            }
            if (&(*iterA) == &(typesigA.back()) && typesigA.back().second == typesigA.front().second)
            {
                if (typesigA.back().first + typesigA.front().first == iterB->first)
                { // backA + frontA = iterB -- is ok, continue
                    *errorpos += iterB->second->size * iterB->first;
                    iA++;
                    iterA = typesigA.begin();
                    iterA++; // start with second
                    iterB++; // continue with next
                    continue;
                }
                if (&(*iterB) == &(typesigB.back()) && typesigB.back().second == typesigB.front().second && typesigA.back().first + typesigA.front().first == typesigB.back().first + typesigB.front().first )
                { // backA + frontA = backB + frontB -- is ok, continue
                // btw: i dont think, this case is ever reached, but for completeness:
                    *errorpos += iterA->second->size * iterA->first + typesigA.front().second->size * typesigA.front().first;
                    iA++;
                    iterA = typesigA.begin();
                    iterA++; // start with second
                    iB++;
                    iterB = typesigB.begin();
                    iterB++; // start with second
                    continue;
                }
                return MUST_ERROR_TYPEMATCH_MISSMATCH; // no match over boundary, so false
            }
            if (&(*iterB) == &(typesigB.back()) && typesigB.back().second == typesigB.front().second)
            {
                if (typesigB.back().first + typesigB.front().first == iterA->first)
                { // backB + frontB = iterA -- is ok, continue
                    *errorpos += iterA->second->size * iterA->first;
                    iB++;
                    iterB = typesigB.begin();
                    iterB++; // start with second
                    iterA++; // continue with next
                    continue;
                }
            }
            // no match over boundary, so false
            *errorpos += std::min(iterA->second->size * iterA->first, iterB->second->size * iterB->first);
            return MUST_ERROR_TYPEMATCH_MISSMATCH; // no match over boundary, so false

        // END handle break condition
        /////////////////////
    }
    return MUST_MESSAGE_NO_ERROR; // type A finished, so subset of type B
}

//=============================
// isASubsetOfB
//=============================
MustMessageIdNames Datatype::isSubsetOfB (
        int countA,
        I_Datatype * typeB,
        int countB,
        MustAddressType* errorpos
        )
{
    *errorpos = 0;
    if (typeB == NULL)
        return MUST_ERROR_TYPEMATCH_INTERNAL_NOTYPE; // type not known
    if (size * countA > typeB->getSize() * countB)
    {
        *errorpos = typeB->getSize() * countB;
        return MUST_ERROR_TYPEMATCH_LENGTH; // size missmatch, cannot be subset
    }
    return checkWhetherSubsetOfB(countA, typeB, countB, errorpos);
}


//=============================
// isAEqualB
//=============================
MustMessageIdNames Datatype::isEqualB (
        int countA,
        I_Datatype * typeB,
        int countB,
        MustAddressType* errorpos
        )
{
    *errorpos = 0;
    if (typeB == NULL)
        return MUST_ERROR_TYPEMATCH_INTERNAL_NOTYPE; // type not known
    if (size * countA < typeB->getSize() * countB)
    {
        *errorpos = size * countA;
        return MUST_ERROR_TYPEMATCH_LENGTH; // size missmatch, cannot be equal
    }
    if (size * countA > typeB->getSize() * countB)
    {
        *errorpos = typeB->getSize() * countB;
        return MUST_ERROR_TYPEMATCH_LENGTH; // size missmatch, cannot be equal
    }
    return checkWhetherSubsetOfB(countA, typeB, countB, errorpos);
}

//=============================
// Free typemap of insignificant bound markers
// MPI_LB will be first, MPI_UB will be last entry of typemap
//=============================
const MustTypemapType& Datatype::getTypemap(){
    int myErr = 0;
    return getTypemap(&myErr);
}

const MustTypemapType& Datatype::getTypemap(int* err){
    if (!cachedTypemap)
    {
#ifdef MUST_DEBUG
        track->CacheMissCount++;
#endif
        cachedTypemap = true;
        *err = 0;
        myTypemap = getFullTypemap(err);
        if (*err != 0) // error on building typemap, do nothing
            return myTypemap;
        if (myHasExplicitLb || myHasExplicitUb)
        { // there are boundmarkers to be striped off before
            MustAddressType lUb=lb+extent, lLb=lb;
            stripBoundmarkersFromTypemap(myTypemap, lLb, lUb);
            if (myHasExplicitLb)
                myTypemap.push_front(std::make_pair(MUST_MPI_LB,lLb));
            if (myHasExplicitUb)
                myTypemap.push_back(std::make_pair(MUST_MPI_UB,lUb));
        }
    }
#ifdef MUST_DEBUG
    else
    {
        track->CacheHitCount++;
    }
#endif
    if (!myIsPredefined && myTypemap.size() == 0)
        *err=1;
    return myTypemap;
}


BlockInfo& Datatype::getBlockInfo()
{
    if (!cachedBlocklist)
    {
#ifdef MUST_DEBUG
        track->CacheMissCount++;
#endif
        getRealBlockInfo();
        MustStridedBlocklistType::iterator iter, nextIter;
        myBlockInfo.checkOverlapped();
//         myBlockInfo.overlapped = isOverlapped(myBlockInfo, iter, nextIter, myBlockInfo.posA, myBlockInfo.posB);
        cachedBlocklist = true;
    }
#ifdef MUST_DEBUG
    else
    {
        track->CacheHitCount++;
    }
#endif
    return myBlockInfo;
}

const MustTypesigType& Datatype::getTypesig(){
    int myErr = 0;
    return getTypesig(&myTypesig, &myErr);
}
const MustTypesig& Datatype::getTypesig(const MustTypesig * const){
    int myErr = 0;
    return getTypesig(&myLongTypesig, &myErr);
}
const MustTypesig& Datatype::getTypesig(const MustTypesig * const , int* err)
{
    if (!cachedLongTypesig)
    {
#ifdef MUST_DEBUG
        track->CacheMissCount++;
#endif
        getRealTypesig(&myLongTypesig, err);
        cachedLongTypesig = true;
    }
#ifdef MUST_DEBUG
    else
    {
        track->CacheHitCount++;
    }
#endif
    return myLongTypesig;
}

const MustTypesigType& Datatype::getTypesig(const MustTypesigType * const ){
    int myErr = 0;
    return getTypesig(&myTypesig, &myErr);
}
const MustTypesigType& Datatype::getTypesig(const MustTypesigType * const , int* err)
{
    if (!cachedTypesig)
    {
#ifdef MUST_DEBUG
        track->CacheMissCount++;
#endif
        if (cachedLongTypesig){
#ifdef MUST_DEBUG
            track->CacheHitCount++;
#endif
            myTypesig.clear();
            MustTypesig::iterator iter;
            for (iter = myLongTypesig.begin(); iter != myLongTypesig.end(); iter++)
            {
                myTypesig.push_back(std::make_pair(iter->first, iter->second->predefValue));
            }
        }
        else
        {
            getRealTypesig(&myTypesig, err);
        }
        cachedTypesig = true;
    }
#ifdef MUST_DEBUG
    else
    {
        track->CacheHitCount++;
    }
#endif
    return myTypesig;
}

bool Datatype::printTypemapString(std::ostream &ss){
    return printTypemapString(ss,MAXTYPEMAP);
}
bool Datatype::printTypemapString(std::ostream &ss, int maxentries){
    int err=0;
    MustTypemapType typemap = getTypemap(&err);
    if (err != 0) // error on building typemap, do nothing
        return false;
    MustTypemapType::iterator iter;

    std::string sep = "";
    ss << "Typemap = {";
    if ( typemap.size() <= maxentries)
    {
        for (iter = typemap.begin(); iter != typemap.end(); iter++)
        {
            ss << sep << "(" << track->getPredefinedName(iter->first) << ", " << iter->second << ")";
            sep = ", ";
        }
    }
    else
    {
        int i;
        MustTypemapType::reverse_iterator riter;
        for (iter = typemap.begin(), i=0; i < maxentries/2 ; iter++, i++)
        {
            ss << sep << "(" << track->getPredefinedName(iter->first) << ", " << iter->second << ")";
            sep = ", ";
        }
        ss << ", ...";
        for (riter = typemap.rbegin(), i=1; i < maxentries/2 ; riter++, i++);
        for (; riter!=typemap.rbegin() ; riter--)
        {
            ss << ", (" << track->getPredefinedName(riter->first) << ", " << riter->second << ")";
        }
        ss << ", (" << track->getPredefinedName(riter->first) << ", " << riter->second << ")";
    }
    ss << "}";// << std::endl;
    return true;
}

void Datatype::stripBoundmarkersFromTypemap(MustTypemapType &typemap, MustAddressType &outLb, MustAddressType &outUb)
{
    bool firstUb=true, firstLb=true;
    MustTypemapType::iterator iter;
    for (iter = typemap.begin(); iter != typemap.end(); iter++){
        while(iter->first == MUST_MPI_LB || iter->first == MUST_MPI_UB){
            if(iter->first == MUST_MPI_LB) // MPI_LB
            {
                if(firstLb){ // first MPI_LB, so init outLb with the position
//                     std::cout << "MPI_LB: " << iter->second << std::endl;
                    outLb = iter->second;
                    firstLb = false;
                }
                else if(outLb > iter->second)
                { // position of currently found MPI_LB is lower then the saved one
//                     std::cout << "MPI_LB: " << iter->second << std::endl;
                    outLb = iter->second;
                }
                iter = typemap.erase(iter); // remove entry from typemap
                if (iter == typemap.end())
                    return;
            }
            else if(iter->first == MUST_MPI_UB) // MPI_UB
            {
                if(firstUb)
                { // first MPI_UB, so init outUb with the position
//                     std::cout << "MPI_UB: " << iter->second << std::endl;
                    outUb = iter->second;
                    firstUb = false;
                }
                else if(outUb < iter->second)
                { // position of currently found MPI_UB is higher then the saved one
//                     std::cout << "MPI_UB: " << iter->second << std::endl;
                    outUb = iter->second;
                }
                iter = typemap.erase(iter); // remove entry from typemap
                if (iter == typemap.end())
                    return;
            }
        }
    }
}

//=============================
// epsilonMagic, does the epsilon magic for extent of datatype
//=============================
void Datatype::epsilonMagic()
{
    // remove old epsilon before calculating the new one!
    this->extent -= this->epsilon;
    this->epsilon = (this->alignment - (this->extent % this->alignment)) % this->alignment;
    this->extent += this->epsilon;
}

//=============================
// getResourceName
//=============================
std::string Datatype::getResourceName (void)
{
    return "Datatype";
}


//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//=============================
// FullBaseTypeInfo
//=============================
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//=============================
// Constructor
//=============================
FullBaseTypeInfo::FullBaseTypeInfo(
        DatatypeTrack *newtrack,
        bool isOptional,
        bool isForReduction,
        bool isBoundMarker,
        bool isNull,
        bool isC,
        bool isFortran,
        bool hasExplicitLb,
        bool hasExplicitUb,
        MustMpiDatatypePredefined predefValue,
        const char* predefName,
        passDatatypePredefinedAcrossP passAcrossFunc)
:       isOptional(isOptional),
        isForReduction(isForReduction),
        isBoundMarker(isBoundMarker),
        predefValue(predefValue),
        predefName(predefName),
        myPassAcrossFunc (passAcrossFunc)
{
            cachedTypesig = false;
            cachedLongTypesig = false;
            cachedTypemap = false;
            cachedBlocklist = false;
            true_lb=lb=0;
            true_extent=extent=size=0; // Will be set correctly later on
            alignment = 1; // Will be set correctly later on
            myIsCommited = false;
            commitPId=0;
            commitLId=0;
            track = newtrack;
            myIsNull=isNull;
            myIsPredefined=!isNull;
            myIsC=isC;
            myIsFortran=isFortran;
            myHasExplicitLb=hasExplicitLb;
            myHasExplicitUb=hasExplicitUb;
}

//=============================
// Generate typemap as used in MPI-standard
//=============================
MustTypemapType FullBaseTypeInfo::getFullTypemap(int* err)
{
    MustTypemapType retval;
    retval.push_back(std::make_pair(predefValue,0));
    return retval;
}

//=============================
// getRealBlockInfo
//=============================
void FullBaseTypeInfo::getRealBlockInfo()
{
    myBlockInfo.overlapped = false;
    myBlockInfo.clear();
    MustAddressType myExtent = extent;
    if (myExtent > 0)
    {
        StridedBlock tempBlock = StridedBlock(myExtent);
        myBlockInfo.insert(tempBlock);
    }
}

//=============================
// getRealTypesig
//=============================
void FullBaseTypeInfo::getRealTypesig(MustTypesig * typesig, int * err)
{
    if (typesig==NULL)
        return;
    *err=0;
    typesig->clear();
    if (!myHasExplicitLb && !myHasExplicitUb)
        typesig->push_back(std::make_pair(1,this));
}
void FullBaseTypeInfo::getRealTypesig(MustTypesigType * typesig, int * err)
{
    if (typesig==NULL)
        return;
    *err=0;
    typesig->clear();
    if (!myHasExplicitLb && !myHasExplicitUb)
        typesig->push_back(std::make_pair(1,predefValue));
}

void FullBaseTypeInfo::setSizes(MustAddressType myExtent, int myAlignment)
{
    true_extent = extent = size = myExtent;
    alignment = myAlignment;
}

//=============================
// getPredefinedName
//=============================
std::string FullBaseTypeInfo::getPredefinedName()
{
    return predefName;
}

std::vector<struct posInfo> FullBaseTypeInfo::posToPath(MustAddressType& errorpos, MustAddressType& pos, MustAddressType& add)
{
    return std::vector<struct posInfo>();
}

bool FullBaseTypeInfo::printRealDatatypePos(std::ostream &out, MustAddressType errorpos){
    return true;
}

bool FullBaseTypeInfo::printRealDatatypeLongPos(std::ostream &out, MustAddressType errorpos){
    out << "(" << predefName << ")";
    return true;
}

bool FullBaseTypeInfo::fillOverlapTree(
    DatatypeForest& f, 
    DatatypeDotNode*& retNode, 
    MustAddressType errorpos, 
    MustAddressType address, 
    int& level,
    int type
)
{
    retNode = f.insertLeafNode(predefName, address);
    level=1;
    return true;
}

bool FullBaseTypeInfo::fillTypemismatchTree(
    DatatypeForest& f, 
    DatatypeDotNode*& retNode, 
    MustAddressType errorpos, 
    MustAddressType pos, 
    int& level,
    int type
)
{
    retNode = f.insertLeafNode(predefName, pos);
    level=1;
    return true;
}

//=============================
// passAcross
//=============================
bool FullBaseTypeInfo::passAcross (
        int rank,
        bool hasHandle,
        MustDatatypeType handle,
        int toPlaceId)
{
    if (!myPassAcrossFunc)
        return false;

    (*myPassAcrossFunc) (
        rank,
        hasHandle,
        handle,
        this->getRemoteId(),
        //
        (int) isOptional,
        (int) isForReduction,
        (int) isBoundMarker,
        (int) myIsNull,
        (int) myIsC,
        (int) myIsFortran,
        (int) myHasExplicitLb,
        (int) myHasExplicitUb,
        (int) predefValue,
        extent,
        alignment,
        //
        toPlaceId
        );

    return true;
}

/*EOF*/
