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
 * @file LeakChecks.cpp
 *       @see MUST::LeakChecks.
 *
 *  @date 17.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "LeakChecks.h"
#include "MustEnums.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(LeakChecks)
mFREE_INSTANCE_FUNCTION(LeakChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(LeakChecks)

#define MAX_NUM_RESSOURCES 100

//=============================
// Constructor
//=============================
LeakChecks::LeakChecks (const char* instanceName)
    : gti::ModuleBase<LeakChecks, I_LeakChecks> (instanceName), myFinCompletion(NULL)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUB_MODULES 9
    if (subModInstances.size() < NUM_SUB_MODULES)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
    if (subModInstances.size() > NUM_SUB_MODULES)
    {
    		for (std::vector<I_Module*>::size_type i = NUM_SUB_MODULES; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myLogger = (I_CreateMessage*) subModInstances[1];
    myCTrack = (I_CommTrack*) subModInstances[2];
    myDTrack = (I_DatatypeTrack*) subModInstances[3];
    myETrack = (I_ErrTrack*) subModInstances[4];
    myGTrack = (I_GroupTrack*) subModInstances[5];
    myKTrack = (I_KeyvalTrack*) subModInstances[6];
	myOTrack = (I_OpTrack*) subModInstances[7];
	myRTrack = (I_RequestTrack*) subModInstances[8];

    //Initialize module data
    /*Nothing to do*/
}

//=============================
// Destructor
//=============================
LeakChecks::~LeakChecks ()
{
	if (myPIdMod)
		destroySubModuleInstance ((I_Module*) myPIdMod);
	myPIdMod = NULL;

	if (myLogger)
		destroySubModuleInstance ((I_Module*) myLogger);
	myLogger = NULL;

	if (myCTrack)
		destroySubModuleInstance ((I_Module*) myCTrack);
	myCTrack = NULL;

	if (myDTrack)
		destroySubModuleInstance ((I_Module*) myDTrack);
	myDTrack = NULL;

	if (myETrack)
		destroySubModuleInstance ((I_Module*) myETrack);
	myETrack = NULL;

	if (myGTrack)
		destroySubModuleInstance ((I_Module*) myGTrack);
	myGTrack = NULL;

	if (myKTrack)
		destroySubModuleInstance ((I_Module*) myKTrack);
	myKTrack = NULL;

	if (myOTrack)
		destroySubModuleInstance ((I_Module*) myOTrack);
	myOTrack = NULL;

	if (myRTrack)
		destroySubModuleInstance ((I_Module*) myRTrack);
	myRTrack = NULL;
}

//=============================
// finalizeNotify
//=============================
GTI_ANALYSIS_RETURN LeakChecks::finalizeNotify (I_ChannelId *thisChannel)
{
	//Is completely reduced without a channel id ? (Reduced on this place)
	if (thisChannel)
	{
		//Initialize completion tree
		if (!myFinCompletion)
			myFinCompletion = new CompletionTree (
					thisChannel->getNumUsedSubIds()-1, thisChannel->getSubIdNumChannels(thisChannel->getNumUsedSubIds()-1));

		myFinCompletion->addCompletion (thisChannel);
	}

	if (!thisChannel || myFinCompletion->isCompleted())
	{
		reportComms ();
		reportDatatypes ();
		reportErrs ();
		reportGroup ();
		reportKeys ();
		reportOps ();
		reportRequests ();
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// reportComms
//=============================
void LeakChecks::reportComms (void)
{
	std::list<std::pair<int, MustCommType> > handles = myCTrack->getUserHandles();

	if (handles.size())
	{
		std::stringstream stream;
		std::list<std::pair<MustParallelId,MustLocationId> > refs;
		std::list<std::pair<int, MustCommType> >::iterator iter;

		stream << "There are " << handles.size() << " communicators that are not freed when MPI_Finalize was issued, a quality application should free all MPI resources before calling MPI_Finalize.";

		if (handles.size () < MAX_NUM_RESSOURCES)
			stream << " Listing information for these communicators:";
		else
			stream << " Listing information for the first " << MAX_NUM_RESSOURCES <<  " communicators:";
		int i = 0;
		for (iter = handles.begin(); iter != handles.end() && i < MAX_NUM_RESSOURCES; iter++, i++)
		{
			int rank = iter->first;
			MustCommType type = iter->second;

			stream << std::endl << std::endl << " -Communicator " << i+1 << ": ";
			I_CommPersistent* comm = myCTrack->getPersistentComm(rank,type);
			comm->printInfo(stream, &refs);
                        comm->erase();
		}

		if (refs.empty())
		    myLogger->createMessage(MUST_ERROR_LEAK_COMM, MustErrorMessage, stream.str(), refs);
		else
		    myLogger->createMessage(MUST_ERROR_LEAK_COMM, refs.front().first, refs.front().second, MustErrorMessage, stream.str(), refs);
	}
}

//=============================
// reportDatatypes
//=============================
void LeakChecks::reportDatatypes (void)
{
	std::list<std::pair<int, MustDatatypeType> > handles = myDTrack->getUserHandles();

	if (handles.size())
	{
		std::stringstream stream;
		std::list<std::pair<MustParallelId,MustLocationId> > refs;
		std::list<std::pair<int, MustDatatypeType> >::iterator iter;

		stream << "There are " << handles.size() << " datatypes that are not freed when MPI_Finalize was issued, a quality application should free all MPI resources before calling MPI_Finalize.";

		if (handles.size () < MAX_NUM_RESSOURCES)
			stream << " Listing information for these datatypes:";
		else
			stream << " Listing information for the first " << MAX_NUM_RESSOURCES <<  " datatypes:";
		int i = 0;
		for (iter = handles.begin(); iter != handles.end() && i < MAX_NUM_RESSOURCES; iter++, i++)
		{
			int rank = iter->first;
			MustDatatypeType type = iter->second;

			stream << std::endl << std::endl << " -Datatype " << i+1 << ": ";
			myDTrack->getDatatype(rank,type)->printInfo(stream, &refs);
		}

		if (refs.empty())
		    myLogger->createMessage(MUST_ERROR_LEAK_DATATYPE, MustErrorMessage, stream.str(), refs);
		else
		    myLogger->createMessage(MUST_ERROR_LEAK_DATATYPE, refs.front().first, refs.front().second, MustErrorMessage, stream.str(), refs);
	}
}

//=============================
// reportErrs
//=============================
void LeakChecks::reportErrs (void)
{
	std::list<std::pair<int, MustErrType> > handles = myETrack->getUserHandles();

	if (handles.size())
	{
		std::stringstream stream;
		std::list<std::pair<MustParallelId,MustLocationId> > refs;
		std::list<std::pair<int, MustErrType> >::iterator iter;

		stream << "There are " << handles.size() << " error handlers that are not freed when MPI_Finalize was issued, a quality application should free all MPI resources before calling MPI_Finalize.";

		if (handles.size () < MAX_NUM_RESSOURCES)
			stream << " Listing information for these error handlers:";
		else
			stream << " Listing information for the first " << MAX_NUM_RESSOURCES <<  " error handlers:";
		int i = 0;
		for (iter = handles.begin(); iter != handles.end() && i < MAX_NUM_RESSOURCES; iter++, i++)
		{
			int rank = iter->first;
			MustErrType type = iter->second;

			stream << std::endl << std::endl << " -Error handler " << i+1 << ": ";
			myETrack->getErr(rank,type)->printInfo(stream, &refs);
		}

		if (refs.empty())
		    myLogger->createMessage(MUST_ERROR_LEAK_ERR, MustErrorMessage, stream.str(), refs);
		else
		    myLogger->createMessage(MUST_ERROR_LEAK_ERR, refs.front().first, refs.front().second, MustErrorMessage, stream.str(), refs);
	}
}

//=============================
// reportGroup
//=============================
void LeakChecks::reportGroup (void)
{
	std::list<std::pair<int, MustGroupType> > handles = myGTrack->getUserHandles();

	if (handles.size())
	{
		std::stringstream stream;
		std::list<std::pair<MustParallelId,MustLocationId> > refs;
		std::list<std::pair<int, MustGroupType> >::iterator iter;

		stream << "There are " << handles.size() << " groups that are not freed when MPI_Finalize was issued, a quality application should free all MPI resources before calling MPI_Finalize.";

		if (handles.size () < MAX_NUM_RESSOURCES)
			stream << " Listing information for these groups:";
		else
			stream << " Listing information for the first " << MAX_NUM_RESSOURCES <<  " groups:";
		int i = 0;
		for (iter = handles.begin(); iter != handles.end() && i < MAX_NUM_RESSOURCES; iter++, i++)
		{
			int rank = iter->first;
			MustGroupType type = iter->second;

			stream << std::endl << std::endl << " -Group " << i+1 << ": ";
			myGTrack->getGroup(rank,type)->printInfo(stream, &refs);
		}

		if (refs.empty())
		    myLogger->createMessage(MUST_ERROR_LEAK_GROUP, MustErrorMessage, stream.str(), refs);
		else
		    myLogger->createMessage(MUST_ERROR_LEAK_GROUP, refs.front().first, refs.front().second, MustErrorMessage, stream.str(), refs);
	}
}

//=============================
// reportKeys
//=============================
void LeakChecks::reportKeys (void)
{
	std::list<std::pair<int, MustKeyvalType> > handles = myKTrack->getUserHandles();

	if (handles.size())
	{
		std::stringstream stream;
		std::list<std::pair<MustParallelId,MustLocationId> > refs;
		std::list<std::pair<int, MustKeyvalType> >::iterator iter;

		stream << "There are " << handles.size() << " keys that are not freed when MPI_Finalize was issued, a quality application should free all MPI resources before calling MPI_Finalize.";

		if (handles.size () < MAX_NUM_RESSOURCES)
			stream << " Listing information for these keys:";
		else
			stream << " Listing information for the first " << MAX_NUM_RESSOURCES <<  " keys:";
		int i = 0;
		for (iter = handles.begin(); iter != handles.end() && i < MAX_NUM_RESSOURCES; iter++, i++)
		{
			int rank = iter->first;
			MustKeyvalType type = iter->second;

			stream << std::endl << std::endl << " -Key " << i+1 << ": ";
			myKTrack->getKeyval(rank,type)->printInfo(stream, &refs);
		}

		if (refs.empty())
		    myLogger->createMessage(MUST_ERROR_LEAK_KEYVAL, MustErrorMessage, stream.str(), refs);
		else
		    myLogger->createMessage(MUST_ERROR_LEAK_KEYVAL, refs.front().first, refs.front().second, MustErrorMessage, stream.str(), refs);
	}
}

//=============================
// reportOps
//=============================
void LeakChecks::reportOps (void)
{
	std::list<std::pair<int, MustOpType> > handles = myOTrack->getUserHandles();

	if (handles.size())
	{
		std::stringstream stream;
		std::list<std::pair<MustParallelId,MustLocationId> > refs;
		std::list<std::pair<int, MustOpType> >::iterator iter;

		stream << "There are " << handles.size() << " operations that are not freed when MPI_Finalize was issued, a quality application should free all MPI resources before calling MPI_Finalize.";

		if (handles.size () < MAX_NUM_RESSOURCES)
			stream << " Listing information for these operation:";
		else
			stream << " Listing information for the first " << MAX_NUM_RESSOURCES <<  " operations:";
		int i = 0;
		for (iter = handles.begin(); iter != handles.end() && i < MAX_NUM_RESSOURCES; iter++, i++)
		{
			int rank = iter->first;
			MustOpType type = iter->second;

			stream << std::endl << std::endl << " -Operation " << i+1 << ": ";
			myOTrack->getOp(rank,type)->printInfo(stream, &refs);
		}

		if (refs.empty())
		    myLogger->createMessage(MUST_ERROR_LEAK_OP, MustErrorMessage, stream.str(), refs);
		else
		    myLogger->createMessage(MUST_ERROR_LEAK_OP, refs.front().first, refs.front().second, MustErrorMessage, stream.str(), refs);
	}
}

//=============================
// reportRequests
//=============================
void LeakChecks::reportRequests (void)
{
	std::list<std::pair<int, MustRequestType> > handles = myRTrack->getUserHandles();

	if (handles.size())
	{
		std::stringstream stream;
		std::list<std::pair<MustParallelId,MustLocationId> > refs;
		std::list<std::pair<int, MustRequestType> >::iterator iter;

		stream << "There are " << handles.size() << " requests that are not freed when MPI_Finalize was issued, a quality application should free all MPI resources before calling MPI_Finalize.";

		if (handles.size () < MAX_NUM_RESSOURCES)
			stream << " Listing information for these requests:";
		else
			stream << " Listing information for the first " << MAX_NUM_RESSOURCES <<  " requests:";
		int i = 0;
		for (iter = handles.begin(); iter != handles.end() && i < MAX_NUM_RESSOURCES; iter++, i++)
		{
			int rank = iter->first;
			MustRequestType type = iter->second;

			stream << std::endl << std::endl << " -Request " << i+1 << ": ";
			myRTrack->getRequest(rank,type)->printInfo(stream, &refs);
		}

		if (refs.empty())
		    myLogger->createMessage(MUST_ERROR_LEAK_REQUEST, MustErrorMessage, stream.str(), refs);
		else
		    myLogger->createMessage(MUST_ERROR_LEAK_REQUEST, refs.front().first, refs.front().second, MustErrorMessage, stream.str(), refs);
	}
}

/*EOF*/
