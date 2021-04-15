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
 * @file QOpCommunication.cpp
 *       @see must::QOpCommunication.
 *
 *  @date 28.02.2013
 *  @author Tobias Hilbrich
 */

#include "QOpCommunication.h"
#include "DWaitState.h"

using namespace must;

//=============================
// QOpCommunication
//=============================
QOpCommunication::QOpCommunication (
        DWaitState *dws,
        MustParallelId pId,
        MustLocationId lId,
        MustLTimeStamp ts,
        I_CommPersistent *comm
)
: QOp (dws, pId, lId, ts),
  myComm(comm)
{

}

//=============================
// ~QOpCommunication
//=============================
QOpCommunication::~QOpCommunication (void)
{
    if (myComm)
        myComm->erase();
    myComm = NULL;
}

//=============================
// getComm
//=============================
I_Comm* QOpCommunication::getComm (void)
{
    return myComm;
}

//=============================
// printVariablesAsLabelString
//=============================
std::string QOpCommunication::printVariablesAsLabelString (void)
{
    std::stringstream stream;
    if (myComm)
    {
        stream << "|comm=";
        if (myComm->isPredefined())
            stream << "MPI_COMM_WORLD"; //not exactly true, but this is for debuging purposes
        else
            stream << myState->getLocationlIdAnalysis()->getInfoForId(myComm->getCreationPId(), myComm->getCreationLId()).callName;
    }
    return QOp::printVariablesAsLabelString() + stream.str();
}

//=============================
// getUsedComms
//=============================
std::list<I_Comm*> QOpCommunication::getUsedComms (void)
{
    std::list<I_Comm*> ret;

    ret.push_back(myComm);

    return ret;
}

//=============================
// hasRequest
//=============================
bool QOpCommunication::hasRequest (void)
{
    return false;
}

//=============================
// getRequest
//=============================
MustRequestType QOpCommunication::getRequest (void)
{
    return 0; /*Invalid value*/
}

/*EOF*/
