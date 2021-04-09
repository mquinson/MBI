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
 * @file I_CompletionCondition.cpp
 *       @see I_CompletionCondition.
 *
 *  @date 16.12.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Fabian Haensel, Joachim Protze
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "CompletionCondition.h"

using namespace must;

mGET_INSTANCE_FUNCTION(CompletionCondition)
mFREE_INSTANCE_FUNCTION(CompletionCondition)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CompletionCondition)

#define TEMP_ARRAY_STEPPING 1000

//=============================
// Constructor
//=============================
CompletionCondition::CompletionCondition (const char* instanceName)
    : gti::ModuleBase<CompletionCondition, I_CompletionCondition> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUB_MODS 1
#if NUM_SUB_MODS>0
    if (subModInstances.size() < NUM_SUB_MODS)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
#endif
    if (subModInstances.size() > NUM_SUB_MODS)
    {
        for (std::vector<I_Module*>::size_type i = NUM_SUB_MODS; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }

    myRTrack = (I_RequestTrack*) subModInstances[0];

    //Initialize module data
    getWrapperFunction ("propagateReducedWait", (GTI_Fct_t*)&myPReducedWait);
    getWrapperFunction ("propagateReducedWaitall", (GTI_Fct_t*)&myPReducedWaitall);
    getWrapperFunction ("propagateReducedWaitsome", (GTI_Fct_t*)&myPReducedWaitsome);
    getWrapperFunction ("propagateReducedWaitany", (GTI_Fct_t*)&myPReducedWaitany);

    myTempArray = new MustRequestType[TEMP_ARRAY_STEPPING];
    myTempArraySize = TEMP_ARRAY_STEPPING;
}

//=============================
// Destructor
//=============================
CompletionCondition::~CompletionCondition ()
{
    myPReducedWaitall = myPReducedWaitsome = myPReducedWaitany = 0;

    if (myRTrack)
        destroySubModuleInstance ((I_Module*) myRTrack);
    myRTrack = NULL;

    if (myTempArray) delete [] myTempArray;
    myTempArraySize = 0;
}

//=============================
// wait
//=============================
GTI_ANALYSIS_RETURN CompletionCondition::wait (
                        MustParallelId pId,
                        MustLocationId lId,
                        MustRequestType request)
{
    I_Request* rInfo = myRTrack->getRequest(pId, request);

    //Is this a helpful (completable) request?
    if (!rInfo || rInfo->isNull() || !rInfo->isActive())
        return GTI_ANALYSIS_SUCCESS;

    if (rInfo->isProcNull())
        return GTI_ANALYSIS_SUCCESS;

    //Create new event
    if (myPReducedWait)
    {
        (*myPReducedWait) (
                pId,
                lId,
                request
        );
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitAny
//=============================
GTI_ANALYSIS_RETURN CompletionCondition::waitAny (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType* requests,
        int count)
{
    //Check size
    checkTempSize(count);

    //Fill the temporary request array
    int numProcNull;
    int usableCount = fillTempArray (count, requests, pId, &numProcNull);

    //Create new event
	if (myPReducedWaitany)
	{
		(*myPReducedWaitany) (
				pId,
				lId,
				myTempArray,
				usableCount,
				numProcNull
			);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitAll
//=============================
GTI_ANALYSIS_RETURN CompletionCondition::waitAll (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType* requests,
        int count)
{
    //Check size
    checkTempSize(count);

    //Fill the temporary request array
    int numProcNull;
    int usableCount = fillTempArray (count, requests, pId, &numProcNull);

    //Create new event
    if (myPReducedWaitall)
    {
        (*myPReducedWaitall) (
                pId,
                lId,
                myTempArray,
                usableCount,
                numProcNull
        );
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// waitSome
//=============================
GTI_ANALYSIS_RETURN CompletionCondition::waitSome (
        MustParallelId pId,
        MustLocationId lId,
        MustRequestType* requests,
        int count)
{
    //Check size
    checkTempSize(count);

    //Fill the temporary request array
    int numProcNull;
    int usableCount = fillTempArray (count, requests, pId, &numProcNull);

    //Create new event
    if (myPReducedWaitsome)
    {
        (*myPReducedWaitsome) (
                pId,
                lId,
                myTempArray,
                usableCount,
                numProcNull
        );
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// checkTempSize
//=============================
void CompletionCondition::checkTempSize (int count)
{
    if (count > myTempArraySize)
    {
        delete myTempArray;
        myTempArraySize = (count / TEMP_ARRAY_STEPPING + 1) * TEMP_ARRAY_STEPPING;
        myTempArray = new MustRequestType[myTempArraySize];
    }
}

//=============================
// fillTempArray
//=============================
int CompletionCondition::fillTempArray (int count, MustRequestType *requests, MustParallelId pId, int *outNumProcNull)
{
    if (outNumProcNull)
        *outNumProcNull = 0;

    int numUsable = 0;
    for (int i = 0; i < count;i++)
    {
        I_Request* rInfo = myRTrack->getRequest(pId, requests[i]);

        //Is this a helpful (completable) request?
        if (!rInfo || rInfo->isNull() || !rInfo->isActive())
            continue;

        if (rInfo->isProcNull())
        {
            if (outNumProcNull)
                *outNumProcNull = *outNumProcNull + 1;

            continue;
        }

        myTempArray[numUsable] = requests[i];
        numUsable++;
    }

    return numUsable;
}

/*EOF*/
