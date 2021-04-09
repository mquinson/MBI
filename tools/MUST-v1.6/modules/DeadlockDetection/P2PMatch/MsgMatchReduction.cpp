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
 * @file MsgMatchReduction.cpp
 *       @see MUST::MsgMatchReduction.
 *
 *  @date 17.03.2011
 *  @author Tobias Hilbrich, Mathias Korepkat
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "MsgMatchReduction.h"

#ifdef MUST_VT
#include <vt_user.h>
#include <dlfcn.h>

void* theHandle = 0;

typedef void (*startP) (const char* name, const char* file, int lno);
typedef void (*endP) (const char* name);

void VT_User_start__(const char* name, const char* file, int lno)
{
	if (!theHandle)
		theHandle = dlopen ("libvt-mpi.so", RTLD_LAZY);

	startP fp = (startP) dlsym (theHandle, "VT_User_start__");
	fp (name, file, lno);
}

void VT_User_end__(const char* name)
{
	endP fp = (endP) dlsym (theHandle, "VT_User_end__");
	fp (name);
}
#endif /*MUST_VT*/

using namespace must;

mGET_INSTANCE_FUNCTION(MsgMatchReduction)
mFREE_INSTANCE_FUNCTION(MsgMatchReduction)
mPNMPI_REGISTRATIONPOINT_FUNCTION(MsgMatchReduction)

//=============================
// Constructor
//=============================
MsgMatchReduction::MsgMatchReduction (const char* instanceName)
    : gti::ModuleBase<MsgMatchReduction, I_MsgMatchReduction> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

#define NUM_SUB_MODS 3

    //handle sub modules
    if (subModInstances.size() < NUM_SUB_MODS)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
    if (subModInstances.size() > NUM_SUB_MODS)
    {
    		for (std::vector<I_Module*>::size_type i = NUM_SUB_MODS; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myLM = (I_P2PMatch*) subModInstances[1];
    myRT = (I_RequestTrack*) subModInstances[2];

    //Initialize module data
}

//=============================
// Destructor
//=============================
MsgMatchReduction::~MsgMatchReduction ()
{
	if (myPIdMod)
		destroySubModuleInstance ((I_Module*) myPIdMod);
	myPIdMod = NULL;

	if (myLM)
		destroySubModuleInstance ((I_Module*) myLM);
	myLM = NULL;

	if (myRT)
		destroySubModuleInstance ((I_Module*) myRT);
	myRT = NULL;

	//free other module data
}

//=============================
// send
//=============================
GTI_ANALYSIS_RETURN MsgMatchReduction::send (
    				MustParallelId pId,
    				MustLocationId lId,
    				int dest,
    				int tag,
    				MustCommType comm,
    				MustDatatypeType type,
    				int count,
    				int mode,
    				gti::I_ChannelId *thisChannel,
    				std::list<gti::I_ChannelId*> *outFinishedChannels)
{
#ifdef MUST_VT
	VT_TRACER("MsgMatchReduction::send");
#endif /*MUST_VT*/

	if (myLM->canOpBeProcessed(pId, comm, dest))
	{
		//Can be processed, filter out
		myLM->send(pId, lId, dest, tag, comm, type, count, mode);
		return GTI_ANALYSIS_SUCCESS;
	}

	//Can't be filtered out
	return GTI_ANALYSIS_IRREDUCIBLE;
}

//=============================
// isend
//=============================
GTI_ANALYSIS_RETURN MsgMatchReduction::isend (
		MustParallelId pId,
		MustLocationId lId,
		int dest,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
		int mode,
		MustRequestType request,
		gti::I_ChannelId *thisChannel,
		std::list<gti::I_ChannelId*> *outFinishedChannels)
{
#ifdef MUST_VT
	VT_TRACER("MsgMatchReduction::isend");
#endif /*MUST_VT*/


	if (myLM->canOpBeProcessed(pId, comm, dest))
	{
		//Can be processed, filter out
		myLM->isend(pId, lId, dest, tag, comm, type, count, mode, request);
		return GTI_ANALYSIS_SUCCESS;
	}

	//Can't be filtered out
	return GTI_ANALYSIS_IRREDUCIBLE;
}

//=============================
// recv
//=============================
GTI_ANALYSIS_RETURN MsgMatchReduction::recv (
		MustParallelId pId,
		MustLocationId lId,
		int source,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
		gti::I_ChannelId *thisChannel,
		std::list<gti::I_ChannelId*> *outFinishedChannels)
{
#ifdef MUST_VT
	VT_TRACER("MsgMatchReduction::recv");
#endif /*MUST_VT*/

	if (myLM->canOpBeProcessed(pId, comm, source))
	{
		//Can be processed, filter out
		myLM->recv(pId, lId, source, tag, comm, type, count);
		return GTI_ANALYSIS_SUCCESS;
	}

	//Can't be filtered out
	return GTI_ANALYSIS_IRREDUCIBLE;
}

//=============================
// irecv
//=============================
GTI_ANALYSIS_RETURN MsgMatchReduction::irecv (
		MustParallelId pId,
		MustLocationId lId,
		int source,
		int tag,
		MustCommType comm,
		MustDatatypeType type,
		int count,
		MustRequestType request,
		gti::I_ChannelId *thisChannel,
		std::list<gti::I_ChannelId*> *outFinishedChannels)
{
#ifdef MUST_VT
	VT_TRACER("MsgMatchReduction::irecv");
#endif /*MUST_VT*/

	if (myLM->canOpBeProcessed(pId, comm, source))
	{
		//Can be processed, filter out
		myLM->irecv(pId, lId, source, tag, comm, type, count, request);
		return GTI_ANALYSIS_SUCCESS;
	}

	//Can't be filtered out
	return GTI_ANALYSIS_IRREDUCIBLE;
}

//=============================
// startPersistent
//=============================
GTI_ANALYSIS_RETURN MsgMatchReduction::startPersistent (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request,
		gti::I_ChannelId *thisChannel,
		std::list<gti::I_ChannelId*> *outFinishedChannels)
{
#ifdef MUST_VT
	VT_TRACER("MsgMatchReduction::startPersistent");
#endif /*MUST_VT*/

	I_Request* info = myRT->getRequest(pId, request);

	if (info == NULL || !info->isPersistent())
		return GTI_ANALYSIS_SUCCESS;

	if (info->isSend())
	{
		if (myLM->canOpBeProcessed(pId, info->getComm(), info->getDest()))
		{
			//Can be processed, filter out
			myLM->startPersistent(pId, lId, request);
			return GTI_ANALYSIS_SUCCESS;
		}
	}
	else
	{
		if (myLM->canOpBeProcessed(pId, info->getComm(), info->getSource()))
		{
			//Can be processed, filter out
			myLM->startPersistent(pId, lId, request);
			return GTI_ANALYSIS_SUCCESS;
		}
	}

	//Can't be filtered out
	return GTI_ANALYSIS_IRREDUCIBLE;
}

//=============================
// timeout
//=============================
void MsgMatchReduction::timeout (void)
{
	//Nothing to do, we never wait
}

/*EOF*/
