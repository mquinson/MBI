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
 * @file GroupChecks.cpp
 *       @see MUST::GroupChecks.
 *
 *  @date 23.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "GroupChecks.h"
#include "MustEnums.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(GroupChecks)
mFREE_INSTANCE_FUNCTION(GroupChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(GroupChecks)

//=============================
// Constructor
//=============================
GroupChecks::GroupChecks (const char* instanceName)
    : gti::ModuleBase<GroupChecks, I_GroupChecks> (instanceName)
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
    myGroupMod = (I_GroupTrack*) subModInstances[4];

    //Initialize module data
    /*None needed*/
}

//=============================
// Destructor
//=============================
GroupChecks::~GroupChecks ()
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

	if (myGroupMod)
		destroySubModuleInstance ((I_Module*) myGroupMod);
	myGroupMod = NULL;
}

//=============================
// errorIfNotKnown
//=============================
GTI_ANALYSIS_RETURN GroupChecks::errorIfNotKnown (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustGroupType group
		)
{
	I_Group* info = myGroupMod->getGroup(pId,group);

	if(info == NULL)
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an unknown group where a valid group was expected.";
		myLogger->createMessage(
				MUST_ERROR_GROUP_UNKNOWN,
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
GTI_ANALYSIS_RETURN GroupChecks::errorIfNull (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustGroupType group
		)
{
	I_Group* info = myGroupMod->getGroup(pId,group);

	if(info && info->isNull())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is null, where a valid group was expected.";
		myLogger->createMessage(
				MUST_ERROR_GROUP_NULL,
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
// errorIfIntegerGreaterGroupSize
//=============================
GTI_ANALYSIS_RETURN GroupChecks::errorIfIntegerGreaterGroupSize (
		MustParallelId pId,
		MustLocationId lId,
		int aId_val,
		int aId_grp,
		int value,
		MustGroupType group
		)
{
	int grpSize = 0;

	I_Group* info = myGroupMod->getGroup(pId,group);
	if(info == NULL || info->isNull())
		return GTI_ANALYSIS_SUCCESS;

	if(!info->isEmpty())
	{
		grpSize = info->getGroup()->getSize();
	}
    // used for the size of the index list (0 <= value <= grpSize)
	if(value > grpSize)
	{
		std::stringstream stream;
		stream
			<< "Argument " << myArgMod->getIndex(aId_val) << " (" << myArgMod->getArgName(aId_val)
			<< ") is greater then the size of the MPI group, which is not allowed. "
			<< "("<<myArgMod->getArgName(aId_val)<<" = "<<value<<" but "<< myArgMod->getArgName(aId_grp)
			<< " is of size: "<<grpSize<<")"
			<< std::endl;

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on " << myArgMod->getArgName(aId_grp) << ": ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
				MUST_ERROR_INTEGER_GREATER_GROUP_SIZE,
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
// errorIfIntegerArrayElementGreaterGroupSize
//=============================
GTI_ANALYSIS_RETURN GroupChecks::errorIfIntegerArrayElementGreaterGroupSize (
		MustParallelId pId,
		MustLocationId lId,
		int aId_val,
		int aId_grp,
		const int* array,
		int size,
		MustGroupType group
		)
{
	bool error = false;
	std::stringstream stream;

	//get size of group
	int grpSize = 0;
	I_Group* info = myGroupMod->getGroup(pId,group);

	if(info == NULL || info->isNull())
		return GTI_ANALYSIS_SUCCESS;

	if( !info->isEmpty() )
	{
		grpSize = info->getGroup()->getSize();
	}

	for( int i=0; i < size; i++ )
	{
        // array contains groupranks (0 <= entry < grpSize)
		if( array[i] >= grpSize )
		{
			if( !error )
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId_val) << " (" << myArgMod->getArgName(aId_val)
					<< ") is an array of ranks that must be in the given MPI group,"
					<< " the following entries do not match this criteria: ";
					error = true;
			}
			else
			{
				stream << ", ";
			}
			
			stream << myArgMod->getArgName(aId_val)<<"["<<i<<"]"<< "=" << array[i];
		}
	}

	if(error)
	{
		stream << "!";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on " << myArgMod->getArgName(aId_grp) << ": ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_ERROR_INTEGER_GREATER_GROUP_SIZE_ARRAY,
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
// errorIfRankFromRangesNotInGroup
//=============================
GTI_ANALYSIS_RETURN GroupChecks::errorIfRankFromRangesNotInGroup (
		MustParallelId pId,
		MustLocationId lId,
		int aId_val,
		int aId_grp,
		const int* array,
		int size,
		MustGroupType group
		)
{

	/*
	 * TODO
	 * This check tests if every computed rank is a valid rank in group.
	 * But openmpi causes an error if the given ranks in the ranges triplets
	 * are not valid.. this is not tested and will not be validated by this check.
	 * e.g.: (0,-10,-15) is valid to this check but will not work with openmpi
	 */

	bool error = false;
	std::stringstream stream;

	//get size of group
	int grpSize = 0;
	I_Group* info = myGroupMod->getGroup(pId,group);
	if( info != NULL && !info->isNull() && !info->isEmpty() )
	{
		grpSize = info->getGroup()->getSize();
	}
	else
	{
		/*
		 * If group is not valid, this check stopps to avoid multiple error
		 * messages on the same problem.
		*/
		return GTI_ANALYSIS_SUCCESS;
	}

	int first, //first rank in triplet
  		last,  //last rank in triplet
		stride, //stride
		X,      // X is +1 if stride > 0
		        //* X is -1 if stride < 0
		num,    //count of ranks in this triplet
		bRank;  // last or first rank in this range

	for( int i=0; i < size; i=i+3 )
	{
		first   = array[i];
		last    = array[i+1];
		stride = array[i+2];

		if(stride == 0)
			continue;
		//get count of ranks in a triplet
		X = 1;
		if (stride < 0) X = -1;
		num = (last - first + X)/stride;

		if (num*stride != last - first + X) //manual ceil
			num++;

		//get second border of this range
		num--; //first rank is already counted
		bRank = (first + num*stride);

		if( bRank >= grpSize || bRank < 0 ||
			 first < 0 || first >= grpSize )
		{
			if( !error )
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId_val) << " (" << myArgMod->getArgName(aId_val)
					<< ") is an array of triplets of the form (first rank, last rank, stride)"
				    << " where all spanned ranks must be in the MPI group ("<< myArgMod->getArgName(aId_grp)<<"),"
					<< " the fellowing triplets do not match this criteria: ";
					error = true;
			}
			else
			{
				stream << ", ";
			}
			stream << myArgMod->getArgName(aId_val)<<"["<<i/3<<"][0-2]";
		}
	}

	if(error)
	{
		stream << "!";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on " << myArgMod->getArgName(aId_grp) << ": ";
		info->printInfo (stream,&refs);
		stream << ")";

		myLogger->createMessage(
			MUST_ERROR_RANK_FROM_RANGES_NOT_IN_GROUP,
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
// warningIfEmpty
//=============================
GTI_ANALYSIS_RETURN GroupChecks::warningIfEmpty (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustGroupType group
		)
{
	I_Group* info = myGroupMod->getGroup(pId,group);

	if(info && info->isEmpty())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is an empty group, which is allowed but unusual.";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;

		stream << "(Information on group: ";
		info->printInfo (stream,&refs);
		stream << ")";


		myLogger->createMessage(
				MUST_WARNING_IF_EMPTY,
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
// warningIfNull
//=============================
GTI_ANALYSIS_RETURN GroupChecks::warningIfNull (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		MustGroupType group
		)
{
	I_Group* info = myGroupMod->getGroup(pId,group);

	if(info && info->isNull())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is MPI_GROUP_NULL, which is allowed but unusual.";

		myLogger->createMessage(
				MUST_WARNING_GROUP_NULL,
				pId,
				lId,
				MustWarningMessage,
				stream.str()
		);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorRankNotInComm
//=============================
GTI_ANALYSIS_RETURN GroupChecks::errorRankNotInComm (
   		MustParallelId pId,
   		MustLocationId lId,
   		int aId_grp,
   		int aId_comm,
   		MustGroupType group,
		MustCommType comm
		)
{
	I_Group* info = myGroupMod->getGroup(pId,group);
	I_Comm* commInfo = myCommMod->getComm(pId,comm);

	if(info == NULL || info->isNull() ||
	   info->isEmpty() || commInfo == NULL ||
	    commInfo->isNull()
	  )
	{
	 return GTI_ANALYSIS_SUCCESS;
	}

	std::stringstream stream;
	bool error = false;
	int grpSize = info->getGroup()->getSize();
	int worldRank,groupRank;

	for( int i = 0; i < grpSize ; i++ )
	{
		//get world rank from group
		info->getGroup()->translate (i, &worldRank);

		//look for worldrank in group of communicator
		if(!commInfo->getGroup()->containsWorldRank (worldRank, &groupRank))
		{
			if( !error )
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId_grp) << " (" << myArgMod->getArgName(aId_grp)
					<< ") is a group which should be a subset of argument " << myArgMod->getIndex(aId_comm) << "(" << myArgMod->getArgName(aId_comm)
					<< "), but the following ranks are in the group but not in the communicator: ";
					error = true;
			}
			else
			{
				stream << ", ";
			}
			
			stream << "Rank in Group: "<<i<<"; Rank in MPI_COMM_WORLD: " << worldRank;
		}
	}

	if(error)
	{
		stream << "!";

		std::list<std::pair<MustParallelId, MustLocationId> > refs;
		stream << "(Information on " << myArgMod->getArgName(aId_grp) << ": ";
		info->printInfo (stream,&refs);
		stream << "; Information on " << myArgMod->getArgName(aId_comm) << ": ";
		commInfo->printInfo (stream,&refs);

		stream << ")";

		myLogger->createMessage(
			MUST_ERROR_RANK_FROM_RANGES_NOT_IN_GROUP,
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
