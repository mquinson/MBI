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
 * @file MessageReproducer.cpp
 *       @see MUST::MessageReproducer.
 *
 *  @date 26.05.2014
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "MessageReproducer.h"

using namespace must;

mGET_INSTANCE_FUNCTION(MessageReproducer)
mFREE_INSTANCE_FUNCTION(MessageReproducer)
mPNMPI_REGISTRATIONPOINT_FUNCTION(MessageReproducer)

//=============================
// Constructor
//=============================
MessageReproducer::MessageReproducer (const char* instanceName)
    : gti::ModuleBase<MessageReproducer, I_MessageReproducer> (instanceName),
      myPIdModule (NULL),
      myLIdModule (NULL)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

#define NUM_SUBS 3
    if (subModInstances.size() < NUM_SUBS)
    {
            std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
            assert (0);
    }
    if (subModInstances.size() > NUM_SUBS)
    {
            for (std::vector<I_Module*>::size_type i = NUM_SUBS; i < subModInstances.size(); i++)
                destroySubModuleInstance (subModInstances[i]);
    }

    //save sub modules
    myPIdModule = (I_ParallelIdAnalysis*) subModInstances[0];
    myLIdModule = (I_LocationAnalysis*) subModInstances[1];
    myLogger = (I_CreateMessage*) subModInstances[2];

    //Read input
    readLog ();
}

//=============================
// Destructor
//=============================
MessageReproducer::~MessageReproducer (void)
{
    if (myPIdModule)
        destroySubModuleInstance ((I_Module*) myPIdModule);
    myPIdModule = NULL;

    if (myLIdModule)
		destroySubModuleInstance ((I_Module*) myLIdModule);
	myLIdModule = NULL;

	if (myLogger)
	    destroySubModuleInstance ((I_Module*) myLogger);
	myLogger = NULL;
}

//=============================
// testForMatch
//=============================
GTI_ANALYSIS_RETURN MessageReproducer::testForMatch (
                        uint64_t pId,
                        uint64_t lId
                        )
{
    KeyType key = std::make_pair(
            std::make_pair(myPIdModule->getInfoForId(pId).rank, myLIdModule->getOccurenceCount(lId)),
            myLIdModule->getInfoForId(pId, lId).callName);

    std::map<KeyType, ValueType>::iterator pos = myTriggers.find (key);

    //If we don't have a hit we simply return
    if (pos == myTriggers.end())
        return GTI_ANALYSIS_SUCCESS;

    //If we have a hit we trigger a message
    myLogger->createMessage(
            0 /*We don't know that one in fact*/,
            pId,
            lId,
            pos->second.second,
            pos->second.first);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// testForMatch
//=============================
void MessageReproducer::readLog (void)
{
    std::ifstream in ("MUST_Output.repro", std::ifstream::in);
    std::string line;

    //Read header line
    std::getline (in, line);

    //Read all lines
    while (std::getline (in, line))
    {
        std::stringstream ls (line);
        std::string values[5];
        int i = 0;

        //Read tokens in line
        while (i < 5 && getline(ls, values[i], ';'))
        {
            i++;
        }

        //Only consider full lines
        if (i != 5)
            continue;

        //Parse out tokens
        int rank = atoi (values[0].c_str());
        std::string callName = values[1];
        long occCount = atol (values[2].c_str());
        std::string text = values[3];
        std::string typeStr = values[4];

        MustMessageType type = MustInformationMessage;
        if (typeStr == "ERROR")
            type = MustErrorMessage;
        else if (typeStr == "Warning")
            type = MustWarningMessage;

        //Add to our triggers
        /**
         * @todo we could thin this our to only include reachable ranks, but the scalability issue of reading a single log file remains anyways (and of the single log to store information on all errors, which might be too many at scale in the first place).
         */
        myTriggers.insert (
            std::make_pair(
                std::make_pair(
                        std::make_pair(rank, occCount),
                        callName
                    ),
                std::make_pair(text, type)
                )
            );
    }

    in.close();
}

/*EOF*/
