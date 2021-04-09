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
 * @file OpTrack.cpp
 *       @see MUST::OpTrack.
 *
 *  @date 10.05.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"

#include "OpTrack.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(OpTrack)
mFREE_INSTANCE_FUNCTION(OpTrack)
mPNMPI_REGISTRATIONPOINT_FUNCTION(OpTrack)

//=============================
// Constructor
//=============================
OpTrack::OpTrack (const char* instanceName)
	: TrackBase<Op, I_Op, MustOpType, MustMpiOpPredefined, OpTrack, I_OpTrack> (instanceName)
{
    //Nothing to do
}

//=============================
// Destructor
//=============================
OpTrack::~OpTrack ()
{
    //Notify HandleInfoBase of ongoing shutdown
    HandleInfoBase::disableFreeForwardingAcross();
}

//=============================
// opCreate
//=============================
GTI_ANALYSIS_RETURN OpTrack::opCreate (
		MustParallelId pId,
		MustLocationId lId,
		int commute,
		MustOpType newOp)
{
	//==Should not be known yet, if so inc mpi ref count (assumed it is neither null nor predefined)
    Op* newInfo = getHandleInfo (pId, newOp);

	if (newInfo)
	{
	    if (!newInfo->isNull() && !newInfo->isPredefined())
	        newInfo->mpiIncRefCount();

		return GTI_ANALYSIS_SUCCESS;
	}

	//==Create the full info
	Op* info = new Op ();

	info->myIsNull = false;
	info->myIsPredefined = false;
	info->myCreationPId = pId;
	info->myCreationLId = lId;
	info->myIsCommutative = (bool) commute;

	//==Store the full info in the user handles
	submitUserHandle (pId, newOp, info);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// opFree
//=============================
GTI_ANALYSIS_RETURN OpTrack::opFree (
		MustParallelId pId,
		MustLocationId lId,
		MustOpType op)
{
	//== Should be known
    removeUserHandle (pId, op);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getOp
//=============================
I_Op* OpTrack::getOp (
		MustParallelId pId,
		MustOpType op)
{
	return getOp (pId2Rank(pId), op);
}

//=============================
// getOp
//=============================
I_Op* OpTrack::getOp (
		int rank,
		MustOpType op)
{
	return getHandleInfo (rank, op);
}

//=============================
// getPersistentOp
//=============================
I_OpPersistent* OpTrack::getPersistentOp (
        MustParallelId pId,
        MustOpType op)
{
    return getPersistentOp (pId2Rank(pId), op);
}

//=============================
// getPersistentOp
//=============================
I_OpPersistent* OpTrack::getPersistentOp (
        int rank,
        MustOpType op)
{
    Op* ret = getHandleInfo (rank, op);
    if (ret) ret->incRefCount();
    return ret;
}

//=============================
// getPredefinedName
//=============================
std::string OpTrack::getPredefinedName (MustMpiOpPredefined predefined)
{
	switch (predefined)
	{
	case MUST_MPI_OP_MAX:
		return "MPI_MAX";
	case MUST_MPI_OP_MIN:
		return "MPI_MIN";
	case MUST_MPI_OP_SUM:
		return "MPI_SUM";
	case MUST_MPI_OP_PROD:
		return "MPI_PROD";
	case MUST_MPI_OP_LAND:
		return "MPI_LAND";
	case MUST_MPI_OP_BAND:
		return "MPI_BAND";
	case MUST_MPI_OP_LOR:
		return "MPI_LOR";
	case MUST_MPI_OP_BOR:
		return "MPI_BOR";
	case MUST_MPI_OP_LXOR:
		return "MPI_LXOR";
	case MUST_MPI_OP_BXOR:
		return "MPI_BXOR";
	case MUST_MPI_OP_MAXLOC:
		return "MPI_MAXLOC";
	case MUST_MPI_OP_MINLOC:
		return "MPI_MINLOC";
	case MUST_MPI_OP_UNKNOWN:
		return "Unknown Operation";
	default:
		std::cerr << "Error: Unknown op enum in " << __FILE__ << ":" << __LINE__ << " check mapping." << std::endl;
		assert (0);
	}

	return "";
}

//=============================
// createPredefinedInfo
//=============================
Op* OpTrack::createPredefinedInfo (int predefEnum, MustOpType handle)
{
    if (handle == myNullValue)
    {
        return new Op();
    }

    MustMpiOpPredefined e = (MustMpiOpPredefined) predefEnum;
    return new Op (e, getPredefinedName(e).c_str());
}

/*EOF*/
