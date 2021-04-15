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
 * @file ErrTrack.cpp
 *       @see MUST::ErrTrack.
 *
 *  @date 12.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "ErrTrack.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(ErrTrack)
mFREE_INSTANCE_FUNCTION(ErrTrack)
mPNMPI_REGISTRATIONPOINT_FUNCTION(ErrTrack)

//=============================
// Constructor
//=============================
ErrTrack::ErrTrack (const char* instanceName)
	: TrackBase<Err, I_Err, MustErrType, MustMpiErrPredefined, ErrTrack, I_ErrTrack> (instanceName)
{
    //Nothing to do
}

//=============================
// Destructor
//=============================
ErrTrack::~ErrTrack ()
{
    //Notify HandleInfoBase of ongoing shutdown
    HandleInfoBase::disableFreeForwardingAcross();
}

//=============================
// errCreate
//=============================
GTI_ANALYSIS_RETURN ErrTrack::errCreate (
		MustParallelId pId,
		MustLocationId lId,
		MustErrType newErr)
{
	//==Should not be known yet
	Err* newInfo = getHandleInfo (pId, newErr);
    if (newInfo)
    {
        if (!newInfo->myIsNull&&!newInfo->myIsPredefined)
            newInfo->mpiIncRefCount();
		return GTI_ANALYSIS_SUCCESS;
    }

	//==Create the full info
	Err* info = new Err ();

	info->myIsNull = false;
	info->myIsPredefined = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;

	//==Store the full info in the user handles
	submitUserHandle (pId, newErr, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errFree
//=============================
GTI_ANALYSIS_RETURN ErrTrack::errFree (
		MustParallelId pId,
		MustLocationId lId,
		MustErrType err)
{
	removeUserHandle (pId, err);
	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getErr
//=============================
I_Err* ErrTrack::getErr (
		MustParallelId pId,
		MustErrType err)
{
	return getErr (pId2Rank(pId), err);
}

//=============================
// getErr
//=============================
I_Err* ErrTrack::getErr (
		int rank,
		MustErrType err)
{
	return getHandleInfo (rank, err);
}

//=============================
// getPersistentErr
//=============================
I_ErrPersistent* ErrTrack::getPersistentErr (
        MustParallelId pId,
        MustErrType err)
{
    return getPersistentErr (pId2Rank(pId), err);
}

//=============================
// getPersistentErr
//=============================
I_ErrPersistent* ErrTrack::getPersistentErr (
        int rank,
        MustErrType err)
{
    Err* info = getHandleInfo (rank, err);
    if (info) info->incRefCount();
    return info;
}

//=============================
// getPredefinedName
//=============================
std::string ErrTrack::getPredefinedName (MustMpiErrPredefined predefined)
{
	switch (predefined)
	{
	case MUST_MPI_ERRORS_ARE_FATAL:
		return "MPI_ERRORS_ARE_FATAL";
	case MUST_MPI_ERRORS_RETURN:
		return "MPI_ERRORS_RETURN";
	case MUST_MPI_ERRORS_UNKNOWN:
		return "Unknown Errorhandler";
	default:
		std::cerr << "Error: Unknown err enum in " << __FILE__ << ":" << __LINE__ << " check mapping." << std::endl;
		assert (0);
	}

	return "";
}

//=============================
// createPredefinedInfo
//=============================
Err* ErrTrack::createPredefinedInfo (int predef, MustErrType handle)
{
    if (handle == myNullValue)
        return new Err ();

    MustMpiErrPredefined e = (MustMpiErrPredefined) predef;
    return  new Err (e, getPredefinedName (e));
}

/*EOF*/
