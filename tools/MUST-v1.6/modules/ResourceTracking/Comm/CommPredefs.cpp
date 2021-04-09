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
 * @file CommPredefs.cpp
 *       @see MUST::CommPredefs
 *
 *  @date 04.03.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "ResourceApi.h"
#include "mustFeaturetested.h"
#include "mustConfig.h"

#include "CommPredefs.h"

#include <mpi.h>
#include <pnmpimod.h>

using namespace must;

mGET_INSTANCE_FUNCTION(CommPredefs)
mFREE_INSTANCE_FUNCTION(CommPredefs)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CommPredefs)

//=============================
// Constructor
//=============================
CommPredefs::CommPredefs (const char* instanceName)
    : gti::ModuleBase<CommPredefs, I_CommPredefs> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    if (subModInstances.size() > 0)
    {
        for (std::vector<I_Module*>::size_type i = 0; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }

    //No sub modules needed

    //Initialize module data
    //Nothing to do
}

//=============================
// Destructor
//=============================
CommPredefs::~CommPredefs ()
{
	//nothing to do
}

//=============================
// propagate
//=============================
GTI_ANALYSIS_RETURN CommPredefs::propagate (
                MustParallelId pId
)
{
	//==Call the API call to propagate the communicator information to I_CommTrack
	propagateCommsP fP;
	if (getWrapperFunction ("propagateComms", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
	{
		int rank, size;
		int err;
		PNMPI_modHandle_t handle;
		PNMPI_Service_descriptor_t service;
		MPI_Comm world = MPI_COMM_WORLD;

		//We need to check whether MPI_COMM_WORLD was splited
		/**
		 * Reenabled this one:
		 * We do this in MPI_Init, my PnMPI versions have recursion guards in MPI_Init
		 * to ensure that we do not wrap MPI calls that an MPI implementation may call
		 * before it issues MPI_Init (as pre of its MPI_Init implementation).
		 * As a result, my PnMPI opts to not do any stacking for the
		 * MPI_Comm_rank/size calls below, thus returning broken stuff.
		 */
#ifdef PNMPI_FIXED
		err = PNMPI_Service_GetModuleByName("split_processes", &handle);
#else
		char string[512];
		sprintf (string, "%s","split_processes");
		err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
		if (err == PNMPI_SUCCESS)
		{
			MPI_Comm thisSetComm;

			err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getMySetComm", "p", &service);
			assert (err == PNMPI_SUCCESS);
			((int(*)(void*)) service.fct) (&thisSetComm);

			world = thisSetComm;
			PMPI_Comm_rank (thisSetComm, &rank);
			PMPI_Comm_size (thisSetComm, &size);
		}
		else
		{
			//No splitting is active, use MPI_COMM_WORLD
			PMPI_Comm_rank (MPI_COMM_WORLD, &rank);
			PMPI_Comm_size (MPI_COMM_WORLD, &size);
 		}

		MustCommType worldF = MUST_Comm_m2i (MPI_COMM_WORLD); //We now use the real MPI_COMM_WORLD here, we should no longer have a chance to see the replaced one (?)

		(*fP) (
            pId,
			rank,
			rank,
			size,
			MUST_Comm_m2i (MPI_COMM_NULL),
			MUST_Comm_m2i (MPI_COMM_SELF),
			MUST_Comm_m2i (MPI_COMM_WORLD), //we must use the real MPI_COMM_WORLD here
			1,
			&worldF
			);
	}

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
