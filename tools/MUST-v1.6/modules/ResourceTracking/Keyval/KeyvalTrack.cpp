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
 * @file KeyvalTrack.cpp
 *       @see MUST::KeyvalTrack.
 *
 *  @date 12.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "KeyvalTrack.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(KeyvalTrack)
mFREE_INSTANCE_FUNCTION(KeyvalTrack)
mPNMPI_REGISTRATIONPOINT_FUNCTION(KeyvalTrack)

//=============================
// Constructor
//=============================
KeyvalTrack::KeyvalTrack (const char* instanceName)
	: TrackBase<Keyval, I_Keyval, MustKeyvalType, MustMpiKeyvalPredefined, KeyvalTrack, I_KeyvalTrack> (instanceName)
{
    //Nothing to do
}

//=============================
// Destructor
//=============================
KeyvalTrack::~KeyvalTrack ()
{
    //Notify HandleInfoBase of ongoing shutdown
    HandleInfoBase::disableFreeForwardingAcross();
}

//=============================
// keyvalCreate
//=============================
GTI_ANALYSIS_RETURN KeyvalTrack::keyvalCreate (
		MustParallelId pId,
		MustLocationId lId,
		MustKeyvalType newKeyval)
{
	//==Should not be known yet
	Keyval* newInfo = getHandleInfo (pId, newKeyval);
    if (newInfo)
    {
        if (!newInfo->myIsNull && !newInfo->myIsPredefined)
            newInfo->mpiIncRefCount();

		return GTI_ANALYSIS_SUCCESS;
    }

	//==Create the full info
	Keyval* info = new Keyval ();

	info->myIsNull = false;
	info->myIsPredefined = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;

	//==Store the full info in the user handles
	submitUserHandle (pId, newKeyval, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// keyvalFree
//=============================
GTI_ANALYSIS_RETURN KeyvalTrack::keyvalFree (
		MustParallelId pId,
		MustLocationId lId,
		MustKeyvalType keyval)
{
    removeUserHandle (pId, keyval);
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getKeyval
//=============================
I_Keyval* KeyvalTrack::getKeyval (
		MustParallelId pId,
		MustKeyvalType keyval)
{
	return getKeyval (pId2Rank(pId), keyval);
}

//=============================
// getKeyval
//=============================
I_Keyval* KeyvalTrack::getKeyval (
		int rank,
		MustKeyvalType keyval)
{
    return getHandleInfo (rank, keyval);
}

//=============================
// getPersistentKeyval
//=============================
I_KeyvalPersistent* KeyvalTrack::getPersistentKeyval (
        MustParallelId pId,
        MustKeyvalType keyval)
{
    return getPersistentKeyval (pId2Rank(pId), keyval);
}

//=============================
// getPersistentKeyval
//=============================
I_KeyvalPersistent* KeyvalTrack::getPersistentKeyval (
        int rank,
        MustKeyvalType keyval)
{
    Keyval* info = getHandleInfo (rank, keyval);
    if (info) info->mpiIncRefCount();
    return info;
}

//=============================
// getPredefinedName
//=============================
std::string KeyvalTrack::getPredefinedName (MustMpiKeyvalPredefined predefined)
{
	switch (predefined)
	{
	case MUST_MPI_KEY_TAG_UB:
		return "MPI_TAG_UB";
	case MUST_MPI_KEY_IO:
		return "MPI_IO";
	case MUST_MPI_KEY_HOST:
		return "MPI_HOST";
	case MUST_MPI_KEY_WTIME_IS_GLOBAL:
		return "MPI_WTIME_IS_GLOBAL";
	case MUST_MPI_KEY_UNKNOWN:
		return "Unknown attribute key";
	default:
		std::cerr << "Error: Unknown keyval enum in " << __FILE__ << ":" << __LINE__ << " check mapping." << std::endl;
		assert (0);
	}

	return "";
}

//=============================
// createPredefinedInfo
//=============================
Keyval* KeyvalTrack::createPredefinedInfo (int predef, MustKeyvalType handle)
{
    if (handle == myNullValue)
        return new Keyval ();

    MustMpiKeyvalPredefined e = (MustMpiKeyvalPredefined) predef;
    return new Keyval (e, getPredefinedName(e));
}

/*EOF*/
