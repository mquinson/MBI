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
 * @file RequestChecks.cpp
 *       @see MUST::RequestChecks.
 *
 *  @date 05.04.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "RequestChecks.h"
#include "MustEnums.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(RequestChecks)
mFREE_INSTANCE_FUNCTION(RequestChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(RequestChecks)

//=============================
// Constructor
//=============================
RequestChecks::RequestChecks (const char* instanceName)
    : gti::ModuleBase<RequestChecks, I_RequestChecks> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBMODULES 4
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
    myReqMod = (I_RequestTrack*) subModInstances[3];

    //Initialize module data
    //Nothing to do
}

//=============================
// Destructor
//=============================
RequestChecks::~RequestChecks ()
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

	if (myReqMod)
		destroySubModuleInstance ((I_Module*) myReqMod);
	myReqMod = NULL;
}

//=============================
// errorIfNotKnown
//=============================
GTI_ANALYSIS_RETURN RequestChecks::errorIfNotKnown (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId, 
	MustRequestType request)
{
	I_Request* info = myReqMod->getRequest(pId, request);

	if (info == NULL)
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a unknown request (neither a predefined nor a user request)!";

		myLogger->createMessage(
				MUST_ERROR_REQUEST_NOT_KNOWN,
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
// errorIfNull
//=============================
GTI_ANALYSIS_RETURN RequestChecks::errorIfNull (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId, 
	MustRequestType request)
{
	I_Request* info = myReqMod->getRequest(pId, request);

	if (info != NULL && info->isNull())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is MPI_REQUEST_NULL!";

		myLogger->createMessage(
				MUST_ERROR_REQUEST_NULL,
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
// warningIfNullorInactive
//=============================
GTI_ANALYSIS_RETURN RequestChecks::warningIfNullOrInactive (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId,
	MustRequestType request)
{
	I_Request* info = myReqMod->getRequest(pId, request);
	std::list<std::pair<MustParallelId, MustLocationId> > refs;

	if (info != NULL &&
		(info->isNull() || !info->isActive()))
	{
		int messageID;
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ")";

		if(info->isNull())
		{
			stream << " is MPI_REQUEST_NULL";
			messageID = MUST_WARNING_REQUEST_NULL;
		}
		else
		{
			stream << " is not active";
			messageID = MUST_WARNING_REQUEST_INACTIVE;
			info->printInfo(stream,&refs);
		}

		stream << " was this intended?";

		myLogger->createMessage(
				messageID,
				pId,
				lId,
				MustWarningMessage,
				stream.str(),
				refs
		);
	}
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNotKnownArray
//=============================
GTI_ANALYSIS_RETURN RequestChecks::errorIfNotKnownArray (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId, 
	MustRequestType* requests, 
	int size)
{
	std::stringstream stream;
	bool error = false;
	I_Request* info;

	//Check all array entries
	for(int i=0;i<size;i++)
	{
		info = myReqMod->getRequest(pId, requests[i]);

		if (info == NULL)
		{
			if(!error)
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") has to be an array of predefined or user defined requests, the following entries are unknown requests: ";
				error = true;
			}
			else
			{
				stream << ", ";
			}
			stream << myArgMod->getArgName(aId)<<"["<<i<<"]";
		}
	}

	if(error)
	{
		stream << ").";
		myLogger->createMessage(
				MUST_ERROR_REQUEST_NOT_KNOWN_ARRAY,
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
// warningIfNullOrInactiveArray
//=============================
GTI_ANALYSIS_RETURN RequestChecks::warningIfNullOrInactiveArray (
	MustParallelId pId, 
	MustLocationId lId,
	int aId,
	MustRequestType* requests, 
	int size)
{
	std::stringstream stream;
	bool warn = true;
	I_Request* info;

	if(size == 0)
		return GTI_ANALYSIS_SUCCESS;

	//Check all array entries
	for(int i=0;i<size;i++)
	{
		info = myReqMod->getRequest(pId, requests[i]);

		if (info != NULL && !info->isNull() && info->isActive())
		{
			warn = false;
			break;
		}
	}

	if(warn)
	{
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an array of requests where all request are either in-active or null, was this intended? ";
		myLogger->createMessage(
				MUST_WARNING_REQUEST_NULL_OR_INACTIVE_ARRAY,
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
// errorIfNullArray
//=============================
GTI_ANALYSIS_RETURN RequestChecks::errorIfNullArray (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId, 
	MustRequestType* requests, 
	int size)
{
	std::stringstream stream;
	bool error = false;
	I_Request* info;

	//Check all array entries
	for(int i=0;i<size;i++)
	{
		info = myReqMod->getRequest(pId, requests[i]);

		if (info != NULL && info->isNull())
		{
			if(!error)
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") has to be an array of user defined requests, however, the following entries are MPI_REQUEST_NULL: ";
				error = true;
			}
			else
			{
				stream << ", ";
			}
			stream << myArgMod->getArgName(aId)<<"["<<i<<"]";
		}
	}

	if(error)
	{
		stream << ").";
		myLogger->createMessage(
				MUST_ERROR_REQUEST_NULL_ARRAY,
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
// errorIfPersistentButInactive
//=============================
GTI_ANALYSIS_RETURN RequestChecks::errorIfPersistentButInactive (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId, 
	MustRequestType request)
{
	I_Request* info = myReqMod->getRequest(pId, request);
	std::list<std::pair<MustParallelId, MustLocationId> > refs;

	if (info != NULL && !info->isNull() && info->isPersistent() && !info->isActive())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a persistent but in-active request! ";

		info->printInfo (stream,&refs);

		myLogger->createMessage(
				MUST_ERROR_REQUEST_PERSISTENT_BUT_INACTIVE,
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
// warningIfCanceled
//=============================
GTI_ANALYSIS_RETURN RequestChecks::warningIfCanceled (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId, 
	MustRequestType request)
{
	I_Request* info = myReqMod->getRequest(pId, request);
	std::list<std::pair<MustParallelId, MustLocationId> > refs;
	
	if (info != NULL && !info->isNull() && info->isCanceled())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") was already marked for cancelation! ";

		info->printInfo (stream,&refs);

		myLogger->createMessage(
				MUST_WARNING_REQUEST_CANCELED,
				pId,
				lId,
				MustWarningMessage,
				stream.str(),
				refs
				);
	}
	return GTI_ANALYSIS_SUCCESS;
}


//=============================
// warningIfActiveRecv
//=============================
GTI_ANALYSIS_RETURN RequestChecks::warningIfActiveRecv (
	MustParallelId pId, 
	MustLocationId lId,
	int aId,
	MustRequestType request)
{
	I_Request* info = myReqMod->getRequest(pId, request);
	std::list<std::pair<MustParallelId, MustLocationId> > refs;
	if (info && info->isActive() && !info->isSend())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an active receive request, that should never be freed as the receiver will"
			<< " have no way to verify that the receive has completed.";

		info->printInfo (stream,&refs);

		myLogger->createMessage(
				MUST_WARNING_REQUEST_ACTIVE_RECV,
				pId,
				lId,
				MustWarningMessage,
				stream.str(),
				refs
				);
	}
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfActive
//=============================
GTI_ANALYSIS_RETURN RequestChecks::errorIfActive (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId, 
	MustRequestType request)
{
	I_Request* info = myReqMod->getRequest(pId, request);
	std::list<std::pair<MustParallelId, MustLocationId> > refs;
	
	if (info != NULL && !info->isNull() && info->isActive())
	{
		std::stringstream stream;

		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is already an active request! ";

		info->printInfo (stream,&refs);

		myLogger->createMessage(
				MUST_ERROR_REQUEST_ACTIVE,
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
// errorIfActiveArray
//=============================
GTI_ANALYSIS_RETURN RequestChecks::errorIfActiveArray (
	MustParallelId pId, 
	MustLocationId lId, 
	int aId, 
	MustRequestType* request, 
	int size)
{
	I_Request* info;
	std::list<std::pair<MustParallelId, MustLocationId> > refs;
	bool error = false;
	std::stringstream stream;

	for(int i=0;i<size;i++)
	{
		info = myReqMod->getRequest(pId, request[i]);

		if (info != NULL && !info->isNull() && info->isActive())
		{
			if(!error)
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") has to be an array of non-active requests, the following entries are already active: ";
				error = true;
			}
			else
			{
				stream << ", ";
			}
			stream << myArgMod->getArgName(aId)<<"["<<i<<"] (";
			info->printInfo (stream,&refs);
			stream << ")";
		}
	}

	if(error)
	{
		myLogger->createMessage(
				MUST_ERROR_REQUEST_ACTIVE_ARRAY,
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

/*EOF*/
