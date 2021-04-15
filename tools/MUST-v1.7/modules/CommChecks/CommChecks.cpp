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
 * @file CommChecks.cpp
 *       @see MUST::CommChecks.
 *
 *  @date 14.04.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "CommChecks.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(CommChecks)
mFREE_INSTANCE_FUNCTION(CommChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CommChecks)

//=============================
// Constructor
//=============================
CommChecks::CommChecks (const char* instanceName)
    : gti::ModuleBase<CommChecks, I_CommChecks> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBMODULES 5
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
    myCommMod = (I_CommTrack*) subModInstances[3];
    myConstMod = (I_BaseConstants*) subModInstances[4];

    //Initialize module data
    //Nothing to do
}

//=============================
// Destructor
//=============================
CommChecks::~CommChecks ()
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

	if (myCommMod)
		destroySubModuleInstance ((I_Module*) myCommMod);
	myCommMod = NULL;

	if (myConstMod)
		destroySubModuleInstance ((I_Module*) myConstMod);
	myConstMod = NULL;
}

//=============================
// errorIfGreaterCommSize
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfGreaterCommSize (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		int value,
		MustCommType comm
		)
{
	//get communicator size
	int commSize = 0;
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull())
	{
		commSize = info->getGroup()->getSize();
	}
	else
	{
		return GTI_ANALYSIS_FAILURE;
	}

	//check value
	if(value > commSize)
	{
	   std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") specifies a rank that is greater then the size of the given communicator. "
			<<"("<< myArgMod->getArgName(aId)  << "=" << value << ", communicator size:"<< commSize<<")!";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_ERROR_INTEGER_GREATER_COMM_SIZE,
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
// errorIfGreaterEqualCommSize
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfGreaterEqualCommSize (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		int value,
		MustCommType comm
		)
{
	//get communicator size
	int commSize = 0;
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull())
	{
		commSize = info->getGroup()->getSize();
	}
	else
	{
		return GTI_ANALYSIS_FAILURE;
	}

	//check value
	if(value >= commSize)
	{
	   std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") specifies a rank that is ";

		if (value == commSize)
			stream << "equal to ";
		else
			stream << "greater then ";

		stream
		    << " the size of the given communicator, while the value must be lower than the size of the communicator. "
			<<"("<< myArgMod->getArgName(aId)  << "=" << value << ", communicator size:"<< commSize<<")!";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_ERROR_INTEGER_GREATER_EQUAL_COMM_SIZE,
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
// errorIfProductGreaterCommSize
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfProductGreaterCommSize (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		const int* array,
		int size,
		MustCommType comm
		)
{
	//get communicator size
	int commSize = 0;
	I_Comm * info = myCommMod->getComm(pId, comm);
	if(info != NULL && !info->isNull())
	{
		commSize = info->getGroup()->getSize();
	}
	else
	{
		return GTI_ANALYSIS_FAILURE;
	}

	//get product of multiplying all ranks in a dims array
	unsigned long long product = 1;
	for(int i=0;i<size;i++)
		product = product * array[i];

	if(product > commSize)
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") specifies a cartesian layout that uses more ranks than available in the given communicator! "
			<<"(Product of dims: " << product << ", communicator size:"<< commSize<<", dims: ";

		for(int i=0;i<size;i++)
		{
			if (i != 0) stream << ", ";
			stream << "[" << i << "]=" << array[i];
		}
		stream << ")";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_ERROR_INTEGER_PRODUCT_GREATER_COMM_SIZE,
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
// warningIfProductLessCommSize
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningIfProductLessCommSize (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		const int* array,
		int size,
		MustCommType comm
		)
{
	//get communicator size
	int commSize = 0;
	I_Comm * info = myCommMod->getComm(pId, comm);
	if(info != NULL && !info->isNull())
	{
		commSize = info->getGroup()->getSize();
	}
	else
	{
		return GTI_ANALYSIS_FAILURE;
	}

	//get product of ranks in a dims array
	unsigned long long product = 1;
	for(int i=0;i<size;i++)
		product = product * array[i];

	if(product < commSize)
	{
	   std::stringstream stream;
	   stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<< ") specifies a cartesian layout that uses less ranks than the given communicator. "
		<< "While this is valid, the remaining ranks will be lost for communication. "
		<<"( specified ranks: " << product << ", communicator size:" << commSize<<", dims: ";

	   for(int i=0;i<size;i++)
	   {
		   if (i != 0) stream << ", ";
		   stream << "[" << i << "]=" << array[i];
	   }
	   stream << ")";

	   std::list<std::pair<MustParallelId, MustLocationId> > refs;
	   stream << "(Information on communicator: ";
	   info->printInfo (stream,&refs);
	   stream << ")";
	   
	   myLogger->createMessage(
				MUST_WARNING_INTEGER_PRODUCT_LESS_COMM_SIZE,
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
// errorIfNotKnown
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfNotKnown (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info == NULL)
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an unknown communicator where a valid communicator was expected.";

		myLogger->createMessage(
				MUST_ERROR_COMM_UNKNWOWN,
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
GTI_ANALYSIS_RETURN CommChecks::errorIfNull (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info && info->isNull())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is MPI_COMM_NULL where a valid communicator was expected.";

		myLogger->createMessage(
				MUST_ERROR_COMM_NULL,
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
// warningIfIsIntercomm
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningIfIsIntercomm (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull() && !info->isPredefined() && info->isIntercomm())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an intercommunicator. Intercommunicators have no toplogy.";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_WARNING_INTER_COMM,
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
// errorIfIsIntercomm
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfIsIntercomm (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull() && !info->isPredefined() && info->isIntercomm())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an intercommunicator and was used where an intracommunicator was expected.";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_ERROR_INTER_COMM,
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
// errorIfNotCart
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfNotCart (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull() && (info->isPredefined() || !info->isCartesian()))
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a communicator with no cartesian topology and was used where such a topology is required.";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_ERROR_NOT_CART_COMM,
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
// warningIfHasTopology
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningIfHasTopology (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(
		(info != NULL && !info->isNull() && !info->isPredefined()) &&
		(info->isCartesian() || info->isGraph())
	   )
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a communicator that already had a process topology.";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_WARNING_NOT_CART_COMM,
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
// errorIfNotGraph
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfNotGraph (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull() && (info->isPredefined() || !info->isGraph()))
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a communicator with no graph topology and was used where such a topology was expected.";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_ERROR_NOT_GRAPH_COMM,
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
// errorIfIsIntercommMPI1
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfIsIntercommMPI1 (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	//TODO there is no test for this check!
	I_Comm * info = myCommMod->getComm(pId, comm);
	if(info != NULL && !info->isNull() && !info->isPredefined() && info->isIntercomm() && myConstMod->isVersion(1))
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an intercommunicator and was used where no intercommunicators are allowed."
			<< "Note that if this was an MPI-2 implementation this would have been allowed, "
			<< "this implementation is of version MPI-"
			<< myConstMod->getVersion()<<"."<<myConstMod->getSubversion() << ".";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_ERROR_INTER_COMM_MPI1,
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
// warningIfIsIntercommMPI2
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningIfIsIntercommMPI2 (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	//TODO there is no test for this check!
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull() && !info->isPredefined() && info->isIntercomm() && myConstMod->getVersion() > 1)
	{
		std::stringstream stream;
		stream
			<< "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an intercommunicator, which is ok in your MPI version, but this is not allowed for implementations that only support MPI-1. (Your MPI version is "
			<< myConstMod->getVersion()<<"."<<myConstMod->getSubversion()<<")";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_WARNING_INTER_COMM_MPI2,
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
// errorIfRootNotInComm
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfRootNotInComm (
		MustParallelId pId,
		MustLocationId lId,
		int aId_root,
		int aId_comm,
		int root,
		MustCommType comm
		)
{
	//get communicator size
	int commSize = 0;
	I_Comm * info = myCommMod->getComm(pId, comm);
	if(info != NULL && !info->isNull())
	{
		commSize = info->getGroup()->getSize();
	}
	else
	{
		return GTI_ANALYSIS_FAILURE;
	}

	//check value
	if(root > commSize)
	{
	    std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId_root) << " (" << myArgMod->getArgName(aId_root)
			<< ") is a rank that is not in the communicator ("<<myArgMod->getArgName(aId_comm)<<"). "
			<<"("<< myArgMod->getArgName(aId_root)  << "=" << root << ", communicator size:"<< commSize<<")!";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_ERROR_ROOT_NOT_IN_COMM,
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
// errorIfIsPredefined
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfIsPredefined (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull() && info->isPredefined())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a predefined communicator, which must not be freed. ";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_ERROR_PREDEFINED_COMM,
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
// errorIfNotIntercomm
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorIfNotIntercomm (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && !info->isNull() && (info->isPredefined() || !info->isIntercomm()))
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an intracommunicator and was used where an intercommunicator was expected.";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_ERROR_NOT_INTER_COMM,
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
// warningIfNull
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningIfNull (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm
		)
{
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL && info->isNull())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is MPI_COMM_NULL, which is allowed but unusual.";

		myLogger->createMessage(
				MUST_WARNING_COMM_NULL,
				pId,
				lId,
				MustWarningMessage,
				stream.str()
		);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// warningMaxDimsGreaterNDims
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningMaxDimsGreaterNDims (
		MustParallelId pId,
		MustLocationId lId,
		int aId_maxdims,
		int aId_comm,
		int maxDims,
		MustCommType comm
		)
{
	//get dimensions of the communicator
	int ndims = 0;
	I_Comm * info = myCommMod->getComm(pId, comm);
	
	if(info != NULL &&
	   !info->isNull() &&
	   !info->isPredefined() &&
	   info->isCartesian()
	   )
	{
		ndims = info->getNdims();
	}
	else
	{
		return GTI_ANALYSIS_SUCCESS;
	}

	//check value
	if(maxDims > ndims)
	{
	   std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId_maxdims) << " (" << myArgMod->getArgName(aId_maxdims)
			<< ") specifies a number of dimension for a cartesian topology, which is greater then the number of dimensions in the given communicator. "
			<<"("<< myArgMod->getArgName(aId_maxdims)  << "=" << maxDims << ", dimensions in communicator:"<< ndims<<")!";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_WARNING_MAXDIMS_GREATER_NDIMS,
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
// errorDirectionGreaterNdims
//=============================
GTI_ANALYSIS_RETURN CommChecks::errorDirectionGreaterNdims (
		MustParallelId pId,
		MustLocationId lId,
		int aId_direction,
		int aId_comm,
		int direction,
		MustCommType comm
		)
{

	//get dimensions of communicator
	int ndims = 0;
	I_Comm * info = myCommMod->getComm(pId, comm);
	if(info != NULL &&
	   !info->isNull() &&
	   !info->isPredefined() &&
	   info->isCartesian()
	   )
	{
		ndims = info->getNdims();
	}
	else
	{
		return GTI_ANALYSIS_SUCCESS;
	}

	//check value
	if(direction > (ndims-1))
	{
	   std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId_direction) << " (" << myArgMod->getArgName(aId_direction)
			<< ") is an index for a dimension to use of a cartesian topology, but is out of range, a number from 0 to ndims-1 was expected. "
			<<"("<< myArgMod->getArgName(aId_direction)  << "=" << direction << ", ndims of communicator="<< ndims<<")!";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_ERROR_DIRECTION_GREATER_NDIMS,
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
// warningMaxNeighborsToSmall
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningMaxNeighborsToSmall (
		MustParallelId pId,
		MustLocationId lId,
		int aId_maxneighbors,
		int aId_rank,
		int aId_comm,
		int maxneighbors,
		int rank,
		MustCommType comm
		)
{
	//get maximal number of neighbors from communicator
	int maxNeighbors_comm = 0;
	I_Comm * info = myCommMod->getComm(pId, comm);
	if(info != NULL &&
	   !info->isNull() &&
	   !info->isPredefined() &&
	   info->isGraph()
	   )
	{

		//if rank is not in communicator stop this check
		if(rank > info->getNnodes() || rank < 0)
		{
			return GTI_ANALYSIS_SUCCESS;
		}
		int pred = rank-1;
		if(pred < 0)
			pred = 0;

		maxNeighbors_comm = info->getIndices()[rank]-info->getIndices()[pred];
	}
	else
	{
		return GTI_ANALYSIS_SUCCESS;
	}

	//check value
	if(maxneighbors < maxNeighbors_comm)
	{
	   std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId_maxneighbors) << " (" << myArgMod->getArgName(aId_maxneighbors)
			<< ") specifies the maximum number of neighbors expected to be retrieved from this call, but is smaller then the actual number of neighbors for this rank. Only partial informations will be returned. "
			<< "(" << myArgMod->getArgName(aId_maxneighbors) << "=" << maxneighbors << ", " << myArgMod->getArgName(aId_rank) << "=" << rank << " and has " << maxNeighbors_comm << " neighbors in the graph topology)";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_WARNING_MAXNEIGHBORS_TO_SMALL,
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
// warningMaxIndicesToSmall
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningMaxIndicesToSmall (
		MustParallelId pId,
		MustLocationId lId,
		int aId_maxindices,
		int aId_comm,
		int maxindices,
		MustCommType comm
		)
{
	//get maximal number of indices from communicator
	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL &&
	   !info->isNull() &&
	   !info->isPredefined() &&
	   info->isGraph()
	   )
	{
		;
	}
	else
	{
		return GTI_ANALYSIS_SUCCESS;
	}

	//check value
	if(maxindices < info->getNnodes())
	{
	   std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId_maxindices) << " (" << myArgMod->getArgName(aId_maxindices)
			<< ") specifies the maximum number of indices to be retrieved for a graph topology, but is smaller then the actual number of indices for the topology defined by the given communicator. Only partial informations will be returned."
			<< "(" << myArgMod->getArgName(aId_maxindices) << "=" << maxindices << " and has " << info->getNnodes() << " indices in the graph topology)";

			std::list<std::pair<MustParallelId, MustLocationId> > refs;
			stream << "(Information on communicator: ";
			info->printInfo (stream,&refs);
			stream << ")";


		myLogger->createMessage(
			MUST_WARNING_MAXINDICES_TO_SMALL,
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
// warningMaxEdgesToSmall
//=============================
GTI_ANALYSIS_RETURN CommChecks::warningMaxEdgesToSmall (
		MustParallelId pId,
		MustLocationId lId,
		int aId_maxedges,
		int aId_comm,
		int maxedges,
		MustCommType comm
		)
{
	//get maximal number of indices from communicator
	int maxedges_comm = 0;

	I_Comm * info = myCommMod->getComm(pId, comm);

	if(info != NULL &&
	   !info->isNull() &&
	   !info->isPredefined() &&
	   info->isGraph()
	   )
	{
		maxedges_comm = info->getIndices()[(info->getNnodes()-1)];
	}
	else
	{
		return GTI_ANALYSIS_SUCCESS;
	}

	//check value
	if(maxedges < maxedges_comm)
	{
	   std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId_maxedges) << " (" << myArgMod->getArgName(aId_maxedges)
			<< ") specifies how many edges may be retrieved for a graph topology, but is smaller then the number of edges in the given communicator. Only partial informations will be returned."
			<< "(" << myArgMod->getArgName(aId_maxedges) << "=" << maxedges << " and has " << maxedges_comm << " edges in the graph topology)";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on communicator: ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_WARNING_MAXEDGES_TO_SMALL,
			pId,
			lId,
			MustWarningMessage,
			stream.str(),
			refs
			);
	}

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/

