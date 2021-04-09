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
 * @file QOpCommunicationCollNonBlocking.cpp
 *       @see must::QOpCommunicationCollNonBlocking.
 *
 *  @date 28.07.2015
 *  @author Tobias Hilbrich
 */

#include "QOpCommunicationCollNonBlocking.h"
#include "DWaitState.h"

using namespace must;

//=============================
// QOpCommunicationCollNonBlocking
//=============================
QOpCommunicationCollNonBlocking::QOpCommunicationCollNonBlocking (
                DWaitState *dws,
                MustParallelId pId,
                MustLocationId lId,
                MustLTimeStamp ts,
                I_CommPersistent *comm,
                MustCollCommType collType,
                MustLTimeStamp waveNumberInComm,
                MustRequestType request
        )
 : QOpCommunicationColl (dws, pId, lId, ts, comm, collType, waveNumberInComm),
   myRequest (request)
{
    //Nothing to do
}

//=============================
// ~QOpCommunicationCollNonBlockingQOpCommunicationCollNonBlocking
//=============================
QOpCommunicationCollNonBlocking::~QOpCommunicationCollNonBlocking (void)
{
    //Nothing to do
}

//=============================
// hasRequest
//=============================
bool QOpCommunicationCollNonBlocking::hasRequest (void)
{
    return true;
}

//=============================
// getRequest
//=============================
MustRequestType QOpCommunicationCollNonBlocking::getRequest (void)
{
    return myRequest;
}

//=============================
// printVariablesAsLabelString
//=============================
std::string QOpCommunicationCollNonBlocking::printVariablesAsLabelString (void)
{
    std::stringstream stream;
    stream << "|request=" << myRequest;

    return QOpCommunicationColl::printVariablesAsLabelString() + stream.str();
}

//=============================
// blocks
//=============================
bool QOpCommunicationCollNonBlocking::blocks (void)
{
    //Non-blocking collective doesn't blocks!
    return false;
}

/*EOF*/
