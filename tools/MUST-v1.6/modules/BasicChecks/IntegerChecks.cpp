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
 * @file IntegerChecks.cpp
 *       @see MUST::IntegerChecks.
 *
 *  @date 01.03.2011
 *  @author Mathias Korepkat
 */

#include "GtiMacros.h"
#include "IntegerChecks.h"
#include "MustEnums.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(IntegerChecks)
mFREE_INSTANCE_FUNCTION(IntegerChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(IntegerChecks)

//=============================
// Constructor
//=============================
IntegerChecks::IntegerChecks (const char* instanceName)
    : gti::ModuleBase<IntegerChecks, I_IntegerChecks> (instanceName)
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
    myConstMod = (I_BaseConstants*) subModInstances[3];

    //Initialize module data
    //Nothing to do
}

//=============================
// Destructor
//=============================
IntegerChecks::~IntegerChecks ()
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

}

//=============================
// errorIfLessThanZero
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfLessThanZero (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		int value)
{
		if(value < 0)
		{
			std::stringstream stream;
			stream
				<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
				<< ") has to be a non-negative integer, but is negative ("
				<< myArgMod->getArgName(aId)  << "=" << value << ")!";

			myLogger->createMessage(
					MUST_ERROR_INTEGER_NEGATIVE,
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
// warningIfZero
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::warningIfZero (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		int value)
{
	if(value == 0)
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is zero, which is correct but unusual!";

		myLogger->createMessage(
				MUST_WARNING_INTEGER_ZERO,
				pId,
				lId,
				MustWarningMessage,
				stream.str()
				);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfZero
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfZero (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		int value)
{
	if(value == 0)
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is zero!";

		myLogger->createMessage(
				MUST_ERROR_INTEGER_ZERO,
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
// warningIfNotOneOrZero
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::warningIfNotOneOrZero (
		MustParallelId pId,
		MustLocationId lId,
		int aId,
		int value)
{
	if(!(value == 0 || value == 1))
	{
		std::stringstream stream;
		stream
			<<  "The logical argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is neither 1 or 0, you specified " << myArgMod->getArgName(aId) << "=" << value << ", which is valid but you might have intended something else here.";

		myLogger->createMessage(
				MUST_WARNING_INTEGER_NOT_ONE_OR_ZERO,
				pId,
				lId,
				MustWarningMessage,
				stream.str()
				);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfLessThanZeroArray
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfLessThanZeroArray (
	MustParallelId pId, 
	MustLocationId lId,
	int aId,
	const int* array,
	int size)
{
	std::stringstream stream;
	bool error = false;
	
	//if array is Null, this would be found by another check.
	if(array == NULL)
		return GTI_ANALYSIS_SUCCESS;

	//Check all array entries
	for(int i=0;i<size;i++)
	{
		if(array[i] < 0)
		{
			if(!error)
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") has to be an array of non-negative integers, the following entrie(s) are negative: ";
			}
			else
			{
				stream << ", ";
			}
			
			stream << myArgMod->getArgName(aId)  << "["<< i<<"]=" << array[i];
				
			error = true;

		}
	}

	//Log if we have an issue
	if(error)
	{
		stream << "!";
		myLogger->createMessage(
				MUST_ERROR_INTEGER_NEGATIVE_ARRAY,
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
// warningIfZeroArray
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::warningIfZeroArray (
	MustParallelId pId, 
	MustLocationId lId,
	int aId,
	const int* array,
	int size)
{
	std::stringstream stream;
	bool error = false;
	
	//if array is Null, this would be found by another check.
	if(array == NULL)
		return GTI_ANALYSIS_SUCCESS;

	//Check all array entries
	for(int i=0;i<size;i++)
	{
		if(array[i] == 0)
		{
			if(!error)
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") is an array that contains zero value(s), which is correct but unusual. (";
			}
			else
			{
				stream << ", ";
			}

			stream << myArgMod->getArgName(aId)  << "["<< i<<"]=" << array[i];
			error = true;
		}
	}

	//Log if we have an issue
	if(error)
	{
		stream << ")";
		myLogger->createMessage(
				MUST_WARNING_INTEGER_ZERO_ARRAY,
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
// errorIfEntryIsGreaterOrEqualArray
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfEntryIsGreaterOrEqualArray (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const int* array,
	int size,
	int border)
{
	std::stringstream stream;
	bool error = false;

	//Check all array entries
	for(int i=0;i<size;i++)
	{
		if(array[i] >= border)
		{
			if(!error)
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") is an array of neighbor nodes for a graph. The graph has " << border 
					<< " nodes, the following entries list higher node indices : ";
			}
			else
			{
				stream << ", ";
			}

			stream << myArgMod->getArgName(aId)  << "["<< i<<"]=" << array[i];
			error = true;
		}
	}

	//Log if we have an issue
	if(error)
	{
		stream << ".";
		myLogger->createMessage(
				MUST_ERROR_INTEGER_ENTRY_GREATER_OR_EQUAL,
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
// warningIfNotOneOrZeroArray
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::warningIfNotOneOrZeroArray (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const int* array,
	int size)
{
	std::stringstream stream;
	bool warning = false;

	//continue if array is null
	if(array == NULL)
		return GTI_ANALYSIS_SUCCESS;
	//Check all array entries
	for(int i=0;i<size;i++)
	{
		if(array[i] != 0 && array[i] != 1)
		{
			if(!warning)
			{
				stream
				<<  "The array of logical arguments " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
				<< ") has entries that are neither 1 or 0, which is valid but you might have intended something else here."
				<< "Non 0 or 1 values are: ";

			}
			else
			{
				stream << ", ";
			}

			stream << myArgMod->getArgName(aId)  << "["<< i<<"]=" << array[i] << " (intepreted as ";
			if (array[i])
				stream << "true";
			else
				stream << "false";
			stream << ")";
			
			warning = true;
		}
	}

	//Log if we have an issue
	if(warning)
	{
		stream << ".";
		myLogger->createMessage(
				MUST_WARNING_INTEGER_NOT_ONE_OR_ZERO_ARRAY,
				pId,
				lId,
				MustWarningMessage,
				stream.str()
				);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNegativNotProcNullAnySource
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfNegativNotProcNullAnySource (
	MustParallelId pId, 
	MustLocationId lId,
	int aId, 
	int value)
{
	if(	value < 0 && 
		!myConstMod->isProcNull(value) && 
		!myConstMod->isAnySource(value)
		)
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") has to be a rank in the given communicator, MPI_PROC_NULL, or MPI_ANY_SOURCE, but is a negative value ("
			<< myArgMod->getArgName(aId)  << "=" << value << ")!";

		myLogger->createMessage(
				MUST_ERROR_INTEGER_NEGATIVE_NOT_PROC_NULL_ANY_SOURCE,
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
// errorIfNegativNotProcNull
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfNegativNotProcNull (
	MustParallelId pId, 
	MustLocationId lId,
	int aId, 
	int value)
{
	if(value < 0 && ! myConstMod->isProcNull(value))
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") has to be a rank in the given communicator or MPI_PROC_NULL, but is negative ("
			<< myArgMod->getArgName(aId)  << "=" << value << ")!";

		myLogger->createMessage(
				MUST_ERROR_INTEGER_NEGATIVE_NOT_PROC_NULL,
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
// errorIfNotWithinRangeZeroAndLessTagUb
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfNotWithinRangeZeroAndLessTagUb (
	MustParallelId pId, 
	MustLocationId lId,
	int aId, 
	int value)
{
	if(value < 0 || value > myConstMod->getTagUb())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a tag, which is outside the range of valid values (0-MPI_TAG_UB("
			<< myConstMod->getTagUb() 
			<<")), but it is "
			<< myArgMod->getArgName(aId)  << "=" << value << "!";

		myLogger->createMessage(
				MUST_ERROR_INTEGER_NOT_WITHIN_ZERO_TAG_UB,
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
// errorIfNotWithinRangeZeroAndLessTagUbAnyTag
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfNotWithinRangeZeroAndLessTagUbAnyTag (
	MustParallelId pId, 
	MustLocationId lId,
	int aId, 
	int value)
{
	if( !myConstMod->isAnyTag(value) &&
		(value < 0 || value > myConstMod->getTagUb()))
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a tag, which is outside the valid range of 0 to MPI_TAG_UB ("<< myConstMod->getTagUb() <<") or MPI_ANY_TAG, but it is "
			<< myArgMod->getArgName(aId)  << "=" << value << "!";

		myLogger->createMessage(
				MUST_ERROR_INTEGER_NOT_WITHIN_ZERO_TAG_UB_ANY_TAG,
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
// warningIfIsHighButLessTagUb
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::warningIfIsHighButLessTagUb (
	MustParallelId pId, 
	MustLocationId lId,
	int aId, 
	int value)
{
	if(value > 32767 && value < myConstMod->getTagUb())
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a tag that is larger than 32767 and may thus only be supported by some MPI implementations."
			<< " This implementation supports tags up to " << myConstMod->getTagUb() 
			<< ". For portability reasons you should check MPI_TAG_UB before using this tag. The specified value was "
			<< myArgMod->getArgName(aId)  << "=" << value << ".";

		myLogger->createMessage(
				MUST_WARNING_INTEGER_HIGH_BUT_LESS_TAG_UB,
				pId,
				lId,
				MustWarningMessage,
				stream.str()
				);
	}

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNegativNotProcNullArray
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfNegativNotProcNullArray (
	MustParallelId pId, 
	MustLocationId lId,
	int aId, 
	const int* array,
	int size)
{
	bool error = false;
	std::stringstream stream;

	for(int i = 0; i<size;i++)
	{
		if(array[i] < 0 && !myConstMod->isProcNull(array[i]))
		{
			if(!error)
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") is an array of rank that must be in the given communicator or MPI_PROC_NULL,"
					<< " the fellowing entries do not match this criteria: ";
				error = true;
			}
			else
			{
				stream << ", ";
			}

			stream << myArgMod->getArgName(aId)<<"["<<i<<"]"<< "=" << array[i];
		}
	}

	if(error)
	{
		stream << "!";
		myLogger->createMessage(
			MUST_ERROR_INTEGER_NEGATIVE_NOT_PROC_NULL_ARRAY,
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
// errorIfNegativProcNullAnySource
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfNegativProcNullAnySource (
	MustParallelId pId, 
	MustLocationId lId,
	int aId, 
	int value)
{
	//TODO MPI-2 standard allows the usage of MPI_PROC_NULL and MPI_ROOT for intercommunicators

	if(	value < 0 || 
		myConstMod->isProcNull(value) || 
		myConstMod->isAnySource(value)
		)
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") is a rank that must be in the given communicator, but it is either negative, MPI_PROC_NULL, or MPI_ANY_SOURCE ("
			<< myArgMod->getArgName(aId)  << "=" << value << ")!";

		myLogger->createMessage(
			MUST_ERROR_INTEGER_NEGATIVE_PROC_NULL_ANY_SOURCE,
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
// errorIfNegativNotUndefined
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfNegativNotUndefined (
	MustParallelId pId, 
	MustLocationId lId,
	int aId, 
	int value)
{
	if(value < 0 && !myConstMod->isUndefined(value))
	{
		std::stringstream stream;
		stream
			<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
			<< ") has to be a non-negative integer or MPI_Undefined but it is: "
			<< myArgMod->getArgName(aId)  << "=" << value << ")!";

		myLogger->createMessage(
			MUST_ERROR_INTEGER_NEGATIVE_UNDEFINED,
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
// errorIfDuplicateRank
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::errorIfDuplicateRank (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const int* array,
	int size)
{
	std::stringstream stream;

	std::map< int , int > entries;

	bool error = false;

	for( int i = 0 ; i < size; i++ )
	{
		if(entries.find(array[i]) != entries.end())
		{
			if(!error)
			{
				stream
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") is an array of ranks where no duplications are allowed,"
					<< " the fellowing entries are duplicated: ";
					error = true;
			}
			else
			{
				stream << ", ";
			}

			stream << myArgMod->getArgName(aId)<<"["<<entries.find(array[i])->second<<"]"
				   << " == " << myArgMod->getArgName(aId)<<"["<<i<<"]"
				   << " == " << array[i];
		}
		else
		{
			entries.insert ( std::pair< int , int >( array[i] , i ) );
		}
	}

	if(error)
	{
		stream << "!";
		myLogger->createMessage(
			MUST_ERROR_INTEGER_DUPLICATION_ARRAY,
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
// checkGroupRangeArray
//=============================
GTI_ANALYSIS_RETURN IntegerChecks::checkGroupRangeArray (
	MustParallelId pId,
	MustLocationId lId,
	int aId,
	const int* array,
	int size)
{
	std::stringstream stream,streamStrideZero;

	std::map< int , int > entries;

	bool error = false;
    bool errorStrideZero = false;
	int first,
		last,
		stride;

	for( int i = 0 ; i < size; i=i+3)
	{
		first = array[i];
		last = array[i+1];
		stride = array[i+2];

		//check for a stride of zero
		if(stride == 0)
		{
			if(!errorStrideZero)
			{
				streamStrideZero
					<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
					<< ") is an array of (from-range, to-range, stride) triplets that contains zero for a stride, which must be greater or smaller than 0. (";
			}
			else
			{
				streamStrideZero << ", ";
			}
			streamStrideZero << myArgMod->getArgName(aId)  << "[" << i/3 << "][2]=" << stride;
			errorStrideZero = true;
			
			//if stride == 0 => we span no ranks and must not run n the overlapp check for this range
			continue;
		}

		//check for overlapping ranges
		std::map<int,bool> hasOverlapps; //stores for which other ranges this range overlapps to avoid reporting the same pair again and again!
		for(int j=first; (j <= last && j >= first) || (j >= last && j <= first); j=j+stride)
		{
			if(entries.find(j) != entries.end())
			{
			    if (hasOverlapps.find (entries[j]) == hasOverlapps.end())
			    {
    					hasOverlapps.insert (std::make_pair(entries[j], true));
			    
					if(!error)
					{
						stream
							<<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
							<< ") is an array of triplets of the form (first rank, last rank, stride)"
							<< " where no duplication of ranks is allowed,"
							<< " the following triplets contain equal ranks: ";
						error = true;
					}
					else
					{
						stream << ", ";
					}

					stream 
						<< myArgMod->getArgName(aId) << "["<<entries.find(j)->second<<"][0-2] overlaps with triplet "
						<< myArgMod->getArgName(aId) << "["<<(i/3)<<"][0-2]";
				}//Overlapp of these two ranges not jet reported ?
			}
			else
			{
				entries.insert ( std::pair< int , int >( j , (i/3) ) );
			}
		}
	} // for triplets

	if(error)
	{
		stream << "!";
		myLogger->createMessage(
			MUST_ERROR_INTEGER_DUPLICATION_ARRAY_TRIPLET,
			pId,
			lId,
			MustErrorMessage,
			stream.str()
			);
	}

	if(errorStrideZero)
	{
		streamStrideZero << ")";
		myLogger->createMessage(
				MUST_ERROR_GROUP_RANGE_STRIDE,
				pId,
				lId,
				MustErrorMessage,
				streamStrideZero.str()
				);
	}

	if(errorStrideZero || error)
	{
		return GTI_ANALYSIS_FAILURE;
	}
	return GTI_ANALYSIS_SUCCESS;
}
/*EOF*/
