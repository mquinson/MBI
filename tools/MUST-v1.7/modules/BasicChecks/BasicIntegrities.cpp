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
 * @file BasicIntegrities.cpp
 *       @see MUST::BasicIntegrities.
 *
 *  @date 10.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "MustDefines.h"
#include "BasicIntegrities.h"
#include <cstdlib>
#include <sstream>

#include <dlfcn.h>

using namespace must;

mGET_INSTANCE_FUNCTION(BasicIntegrities)
mFREE_INSTANCE_FUNCTION(BasicIntegrities)
mPNMPI_REGISTRATIONPOINT_FUNCTION(BasicIntegrities)

//=============================
// Constructor
//=============================
BasicIntegrities::BasicIntegrities (const char* instanceName)
    : gti::ModuleBase<BasicIntegrities, I_BasicIntegrities> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUB_MODULES 5
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
    myArgMod = (I_ArgumentAnalysis*) subModInstances[2];
    myConstMod = (I_BaseConstants*) subModInstances[3];
    myCommMod = (I_CommTrack*) subModInstances[4];
    //Initialize module data
    /*Nothing to do*/
}

//=============================
// Destructor
//=============================
BasicIntegrities::~BasicIntegrities ()
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


	if (myConstMod)
		destroySubModuleInstance ((I_Module*) myConstMod);
	myConstMod = NULL;

	if (myCommMod)
		destroySubModuleInstance ((I_Module*) myCommMod);
	myCommMod = NULL;
}

//=============================
// errorIfNullCondition
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullCondition (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	int size,
	const void* pointer)
{

	if(	size > 0 && pointer == NULL)
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<<  ") is a NULL pointer where an allocated memory region with a size of " 
		<<  size << " byte was expected!";

		myLogger->createMessage(
				MUST_ERROR_POINTER_NULL,
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
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNull (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const void* pointer)
{
	return errorIfNullCondition(pId,lId,aId,1,pointer);
}

//=============================
// errorIfNullAndNotMpiBottom
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullAndNotMpiBottom (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	int size,
	const void* pointer)
{
	if(	size > 0 && (pointer == NULL && pointer != myConstMod->getBottom()))
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<<  ") is a NULL pointer where an allocated memory region with a size of " 
		<<  size << " byte was expected!";

		myLogger->createMessage(
				MUST_ERROR_POINTER_NULL_NOT_BOTTOM,
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
// errorIfNullCommSize
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullCommSize (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	MustCommType comm,
	const void* pointer)
{

	int commSize = 0;
	I_Comm* info = myCommMod->getComm(pId,comm);
	if(info != NULL && !info->isNull())
	{
		commSize = info->getGroup()->getSize();
	}
	else
	{
		return GTI_ANALYSIS_SUCCESS;
	}

	if(	commSize > 0 && pointer == NULL)
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<<  ") is a NULL pointer where an allocated memory region with a size of " 
		<<  commSize << " byte was expected!";

		myLogger->createMessage(
				MUST_ERROR_POINTER_NULL_COMM_SIZE,
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
// errorIfNullAndNotMpiBottomConditionCommSize
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullAndNotMpiBottomConditionCommSize (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const int* array,
	MustCommType comm,
	const void* pointer)
{
	//if array is null continue
	if(array == NULL)
		return GTI_ANALYSIS_SUCCESS;

	int commSize = 0;
	I_Comm* info = myCommMod->getComm(pId,comm);
	if(info != NULL && !info->isNull())
	{
		commSize = info->getGroup()->getSize();
	}
	else
	{
		return GTI_ANALYSIS_SUCCESS;
	}

	int sumRanks = 0;
	for(int i=0;i<commSize;i++)
	{
		sumRanks = sumRanks + array[i];
	}

	if(	commSize > 0 &&
		sumRanks > 0 &&
		pointer == NULL &&
		pointer != myConstMod->getBottom()
	  )
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<< ") is a NULL pointer where an allocated memory region was expected!";

		myLogger->createMessage(
				MUST_ERROR_POINTER_NULL_COMM_SIZE_ARRAY,
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
// warningIfNull
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::warningIfNull (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const void* pointer)
{
	if(	pointer == NULL )
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<< ") is a NULL pointer,  which is allowed but unusual.";

		myLogger->createMessage(
				MUST_WARNING_POINTER_NULL,
				pId,
				lId,
				MustWarningMessage,
				stream.str()
			);
	}

	return GTI_ANALYSIS_SUCCESS;

}

//=============================
// warningForLowThreadlevel
//=============================
#pragma weak omp_get_max_threads
extern "C" int omp_get_max_threads(void);

GTI_ANALYSIS_RETURN BasicIntegrities::warningForLowThreadlevel (
	MustParallelId pId,
	MustLocationId lId,
	int requested,
	int provided)
{
    bool omp_used = false;
#ifdef RTLD_DEFAULT
    // Try to resolve omp_get_max_threads via dlsym
    void *addr = dlsym(RTLD_DEFAULT, "omp_get_max_threads");
    omp_used = (addr != NULL);
#else
    // See if omp_get_max_threads was resolved by the
    // dynamic loader.
    //char *omp_num_str = getenv("OMP_NUM_THREADS");
    omp_used = ((unsigned long)omp_get_max_threads != NULL);
#endif
	if ( omp_used )
	{
                int threads = omp_get_max_threads();
                //printf("DEBUG: omp_get_max_threads() => %i\n", threads);
		//int threads = atoi(getenv("OMP_NUM_THREADS"));
		/**
		 * @todo We must not use "magic"+unportable numbers here,
		 *            the constants for the different thread levels need to be 
		 *            added to the base constants module.
		 *            This impacts the "0" below and the if-switch below
		 */
		if (threads < 2 || (provided > 0 && requested != 0))
		{
			return GTI_ANALYSIS_SUCCESS;
		}
		std::stringstream stream;
		
		// TODO: Get the threadlevels from constants storage
		// TODO: Use MPI_Query_thread to get the provided threadlevel.#
		
		if (requested==0 && provided==1) // MPI_Init hard wired, as not allowed by MPI standard?
		{
			stream
			<<  "You requested " << threads << " threads by OMP_NUM_THREADS "
			<<  "but used MPI_Init to start your application. This is ok if "
			<<  "your MPI library supports threads or your application doesn't "
			<<  "use any OpenMP. The standard encourages you to use MPI_Init_thread "
			<<  "when using threads in MPI applications.";
		}
		else if (requested==MPI_THREAD_SINGLE)
		{
			stream
			<<  "You requested " << threads << " threads by OMP_NUM_THREADS "
			<<  "but requested MPI_THREAD_SINGLE from the mpi library. This is ok "
			<<  "as long as your application doesn't use any OpenMP before "
			<<  "MPI_Finalize.";
		}
		else
		{
			stream
			<<  "You requested " << threads << " threads by OMP_NUM_THREADS "
			<<  "and requested thread level "; 
                        if (requested == MPI_THREAD_SINGLE)
                            stream << "MPI_THREAD_SINGLE";
			else if (requested == MPI_THREAD_FUNNELED)
                            stream << "MPI_THREAD_FUNNELED";
                        else if (requested == MPI_THREAD_SERIALIZED)
                            stream << "MPI_THREAD_SERIALIZED";
                        else if (requested == MPI_THREAD_MULTIPLE)
                            stream << "MPI_THREAD_MULTIPLE";
			stream << " from the mpi library but the library provides no thread support."
			<< "This is ok as long as your application doesn't make use of OpenMP between "
			<<  "MPI_Init and MPI_Finalize.";
		}

		myLogger->createMessage(
				MUST_WARNING_THREADLEVEL,
				pId,
				lId,
				MustWarningMessage,
				stream.str()
			);
	}

	return GTI_ANALYSIS_SUCCESS;

}

//=============================
// errorIfNullAndNotMpiBottomAtIndexCommSize
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullAndNotMpiBottomAtIndexCommSize (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const int* array,
	MustCommType comm,
	const void* pointer)
{

	//get communicator size
	int commSize = 0;
	I_Comm* info = myCommMod->getComm(pId,comm);
	if(info != NULL && !info->isNull())
	{
		commSize = info->getGroup()->getSize();
	}
	else
	{
		return GTI_ANALYSIS_SUCCESS; //stop if communicator is not valid or useful for this check
	}

	//get rank in specified communicator
	int groupRank=0;
	int worldRank = myPIdMod->getInfoForId(pId).rank;
	if(!info->getGroup()->containsWorldRank (worldRank, &groupRank))
		return GTI_ANALYSIS_SUCCESS; //stop this check if rank is notin communicator


	if(	commSize > 0 &&
		array[groupRank] > 0 &&
		pointer == NULL &&
		pointer != myConstMod->getBottom()
	  )
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<< ") is a NULL pointer where an allocated memory region was expected!";

		myLogger->createMessage(
				MUST_ERROR_POINTER_NULL_COMM_SIZE_ARRAY_AT_INDEX,
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
// errorIfNullStatus
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullStatus (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const void* pointer)
{
	//If we have an MPI_STATUS_IGNORE, check whether it is one
#ifdef HAVE_MPI_STATUS_IGNORE
	if (pointer == MPI_STATUS_IGNORE) /*IMPORTANT: as this is an integrity, we can refer to MPI constants here*/
		return GTI_ANALYSIS_SUCCESS;
#endif /*HAVE_MPI_STATUS_IGNORE*/

	if(pointer == NULL)
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<< ") is a NULL pointer where an allocated memory region was expected!";

		myLogger->createMessage(
				MUST_ERROR_POINTER_NULL_STATUS_IGNORE,
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
// errorIfNullStatuses
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullStatuses (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const void* pointer)
{
	//If we have an MPI_STATUSES_IGNORE, check whether it is one
#ifdef HAVE_MPI_STATUSES_IGNORE
	if(pointer == MPI_STATUSES_IGNORE) /*IMPORTANT: as this is an integrity, we can refer to MPI constants here*/
		return GTI_ANALYSIS_SUCCESS;
#endif /*HAVE_MPI_STATUS_IGNORE*/

	if (pointer == NULL)
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<< ") is a NULL pointer where an allocated memory region was expected!";

		myLogger->createMessage(
				MUST_ERROR_POINTER_NULL_STATUS_IGNORE,
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
// errorIfNullAndNotMpiBottomOnlyOnRoot
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullAndNotMpiBottomOnlyOnRoot (
    	    		MustParallelId pId,
    	    		MustLocationId lId,
    	    		int aId,
    	    		int size,
    	    		const void* pointer,
    	    		int root,
    	    		MustCommType comm
    				)
{
	I_Comm* info = myCommMod->getComm(pId,comm);
	if(info == NULL || info->isNull())
	{
		return GTI_ANALYSIS_SUCCESS; //stop if communicator is not valid or useful for this check
	}

	//get rank in specified communicator
	int worldRank = myPIdMod->getInfoForId(pId).rank,
		worldroot;

	if (!info->getGroup()->translate(root, &worldroot))
	{
		return GTI_ANALYSIS_SUCCESS;
	}

	if( worldRank == worldroot &&
		size > 0 &&
		    (
		    	pointer == NULL &&
		    	pointer != myConstMod->getBottom()
		     )
	   )
	{
			std::stringstream stream;
			stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a NULL pointer where an allocated memory region was expected!";

			myLogger->createMessage(
					MUST_ERROR_POINTER_NULL_NOT_BOTTOM,
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
// errorIfNullOnlyOnRoot
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullOnlyOnRoot (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		const void* pointer,
		int root,
		MustCommType comm
		)
{
	I_Comm* info = myCommMod->getComm(pId,comm);
	if(info == NULL || info->isNull())
	{
		return GTI_ANALYSIS_SUCCESS; //stop if communicator is not valid or useful for this check
	}

	//get rank in specified communicator
	int worldRank = myPIdMod->getInfoForId(pId).rank,
			worldroot;
	info->getGroup()->translate(root, &worldroot);


	if( worldRank == worldroot)
	{
		return errorIfNullCondition(pId,lId,aId,1,pointer);
	}
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNullAndNotMpiBottomConditionCommSizeOnlyOnRoot
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullAndNotMpiBottomConditionCommSizeOnlyOnRoot (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		const int* array,
		MustCommType comm,
		const void* pointer,
		int root
		)
{

	I_Comm* info = myCommMod->getComm(pId,comm);
	if(info == NULL || info->isNull())
	{
		return GTI_ANALYSIS_SUCCESS; //stop if communicator is not valid or useful for this check
	}

	//get rank in specified communicator
	int worldRank = myPIdMod->getInfoForId(pId).rank,
			worldroot;
	info->getGroup()->translate(root, &worldroot);

	if( worldRank == worldroot)
	{
		return errorIfNullAndNotMpiBottomConditionCommSize (
				pId,
				lId,
				aId,
				array,
				comm,
				pointer
				);
	}
	return GTI_ANALYSIS_SUCCESS;
}
//=============================
// errorIfNullCommSizeOnlyOnRoot
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfNullCommSizeOnlyOnRoot (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm,
		const void* pointer,
		int root)
{
	I_Comm* info = myCommMod->getComm(pId,comm);
	if(info == NULL || info->isNull())
	{
		return GTI_ANALYSIS_SUCCESS; //stop if communicator is not valid or useful for this check
	}

	//get rank in specified communicator
	int worldRank = myPIdMod->getInfoForId(pId).rank,
			worldroot;
	info->getGroup()->translate(root, &worldroot);


	if( worldRank == worldroot)
	{
		int commSize = info->getGroup()->getSize();
		if(	commSize > 0 && pointer == NULL)
		{
			std::stringstream stream;
			stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a NULL pointer where an allocated memory region was expected!";

			myLogger->createMessage(
					MUST_ERROR_POINTER_NULL_COMM_SIZE,
					pId,
					lId,
					MustErrorMessage,
					stream.str()
				);

			return GTI_ANALYSIS_FAILURE;
		}
	}
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfInPlaceOtherThanRoot
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfInPlaceOtherThanRoot (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustCommType comm,
		MustAddressType pointer,
		int root)
{
	I_Comm* info = myCommMod->getComm(pId,comm);
	if(info == NULL || info->isNull())
	{
		return GTI_ANALYSIS_SUCCESS; //stop if communicator is not valid or useful for this check
	}

	//get rank in specified communicator
	int worldRank = myPIdMod->getInfoForId(pId).rank,
			worldroot;
	info->getGroup()->translate(root, &worldroot);


	if( worldRank != worldroot)
	{
		if( pointer == MUST_IN_PLACE)
		{
			std::stringstream stream;
			stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is MPI_IN_PLACE where it is not allowed!";

			myLogger->createMessage(
					MUST_ERROR_MPI_IN_PLACE_USED,
					pId,
					lId,
					MustErrorMessage,
					stream.str()
				);

			return GTI_ANALYSIS_FAILURE;
		}
	}
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfInPlaceUsed
//=============================
GTI_ANALYSIS_RETURN BasicIntegrities::errorIfInPlaceUsed (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustAddressType pointer)
{
	if( pointer == MUST_IN_PLACE)
	{
		std::stringstream stream;
		stream
		<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
		<< ") is MPI_IN_PLACE where it is not allowed!";

		myLogger->createMessage(
				MUST_ERROR_MPI_IN_PLACE_USED,
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
