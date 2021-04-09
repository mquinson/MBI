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
 * @file QOp.cpp
 *       @see must::QOp.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "QOp.h"
#include "DWaitState.h"

#include <assert.h>

#include <sstream>
#include <fstream>

using namespace must;

//=============================
// QOp
//=============================
QOp::QOp (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts
        )
 : myState (dws),
   myPId (pId),
   myLId (lId),
   myTS (ts),
   myRank (-1),
   myRefCount (1)
{
    assert(dws);
    myRank = dws->getParallelIdAnalysis()->getInfoForId(pId).rank;
}

//=============================
// QOp
//=============================
QOp::~QOp ()
{

}

//=============================
// getIssuerRank
//=============================
int QOp::getIssuerRank ()
{
    return myRank;
}

//=============================
// getPId
//=============================
MustParallelId QOp::getPId (void)
{
    return myPId;
}

//=============================
// getLId
//=============================
MustLocationId QOp::getLId (void)
{
    return myLId;
}

//=============================
// getTimeStamp
//=============================
MustLTimeStamp QOp::getTimeStamp (void)
{
    return myTS;
}

//=============================
// incRefCount
//=============================
int QOp::incRefCount(void)
{
    myRefCount++;
    return myRefCount;
}

//=============================
// erase
//=============================
int QOp::erase ()
{
    myRefCount--;
    int temp = myRefCount;

    if (myRefCount == 0)
        delete (this);

    return temp;
}


//=============================
// printAsDot
//=============================
std::string QOp::printAsDot (std::ofstream &out, std::string nodePrefix, std::string color)
{
    std::string nodeName = nodePrefix + "_op";
    out
        << nodeName << "[label=\"{" << printVariablesAsLabelString() << "}\", shape=Mrecord, fillcolor=" << color << ", style=filled" << "];" << std::endl;

    return nodeName;
}

//=============================
// printVariablesAsLabelString
//=============================
std::string QOp::printVariablesAsLabelString (void)
{
    std::stringstream stream;
    stream << myState->getLocationlIdAnalysis()->getInfoForId(myPId, myLId).callName << "|refCount=" << myRefCount;
    return stream.str();
}

//=============================
// asOpCommunicationP2P
//=============================
QOpCommunicationP2P* QOp::asOpCommunicationP2P (void)
{
    return NULL;
}

//=============================
// asOpCommunicationColl
//=============================
QOpCommunicationColl* QOp::asOpCommunicationColl (void)
{
    return NULL;
}

//=============================
// getPingPongNodes
//=============================
std::set<int> QOp::getPingPongNodes (void)
{
    std::set<int> ret;
    return ret;
}

/*EOF*/
