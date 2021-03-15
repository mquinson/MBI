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
 * @file OperationChecks.cpp
 *       @see MUST::OperationChecks.
 *
 *  @date 26.05.2011
 *  @author Mathias Korepkat
 */

#include "GtiMacros.h"
#include "OperationChecks.h"
#include "MustEnums.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(OperationChecks)
mFREE_INSTANCE_FUNCTION(OperationChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(OperationChecks)

//=============================
// Constructor
//=============================
OperationChecks::OperationChecks (const char* instanceName)
    : gti::ModuleBase<OperationChecks, I_OperationChecks> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODULES 4
    if (subModInstances.size() < NUM_MODULES)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
    if (subModInstances.size() > NUM_MODULES)
    {
    		for (std::vector<I_Module*>::size_type i = NUM_MODULES; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myLogger = (I_CreateMessage*) subModInstances[1];
    myArgMod = (I_ArgumentAnalysis*) subModInstances[2];
    myOpMod = (I_OpTrack*) subModInstances[3];

    //Initialize module data
    //Nothing to do
}

//=============================
// Destructor
//=============================
OperationChecks::~OperationChecks ()
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

	if (myOpMod)
		destroySubModuleInstance ((I_Module*) myOpMod);
	myOpMod = NULL;

}

//=============================
// errorIfPredefined
//=============================
GTI_ANALYSIS_RETURN OperationChecks::errorIfPredefined (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustOpType op)
{
	I_Op* info = myOpMod->getOp(pId,op);
	//continue if operation is unknown
	if(info == NULL)
		return GTI_ANALYSIS_SUCCESS;
	if( info->isPredefined() )
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a predefined operation where a user-defined operation was expected. (Operation: ";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		info->printInfo(stream, &refs);
		stream << ")";

		myLogger->createMessage(
			MUST_ERROR_OPERATION_PREDEFINED,
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
// errorIfNotKnown
//=============================
GTI_ANALYSIS_RETURN OperationChecks::errorIfNotKnown (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustOpType op)
{
	I_Op* info = myOpMod->getOp(pId,op);

	if( info == NULL )
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a unknown operation where a valid operation was expected.";

		myLogger->createMessage(
			MUST_ERROR_OPERATION_UNKNOWN,
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
GTI_ANALYSIS_RETURN OperationChecks::errorIfNull (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustOpType op)
{
	I_Op* info = myOpMod->getOp(pId,op);

	//continue if operation is unknown
	if(info == NULL)
		return GTI_ANALYSIS_SUCCESS;

	if( info->isNull() )
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is MPI_OP_NULL where a valid operation was expected.";

		myLogger->createMessage(
			MUST_ERROR_OPERATION_NULL,
			pId,
			lId,
			MustErrorMessage,
			stream.str()
		);

		return GTI_ANALYSIS_FAILURE;
	}

	return GTI_ANALYSIS_SUCCESS;
}
/*EOF*/
