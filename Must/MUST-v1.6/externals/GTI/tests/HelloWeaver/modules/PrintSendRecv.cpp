/* This file is part of GTI (Generic Tool Infrastructure)
 *
 * Copyright (C)
 *  2008-2019 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2008-2019 Lawrence Livermore National Laboratories, United States of America
 *  2013-2019 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file PrintSendRecv.cpp
 *       @see gti::PrintSendRecv.
 *
 *  @date 26.01.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"

#include "PrintSendRecv.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(PrintSendRecv)
mFREE_INSTANCE_FUNCTION(PrintSendRecv)
mPNMPI_REGISTRATIONPOINT_FUNCTION(PrintSendRecv)

//=============================
// Constructor
//=============================
PrintSendRecv::PrintSendRecv (const char* instanceName)
    : ModuleBase<PrintSendRecv, I_PrintSendRecv> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    assert (subModInstances.empty()); //Needs no sub modules ...
}

//=============================
// Destructor
//=============================
PrintSendRecv::~PrintSendRecv ()
{
    //Nothing to do
}

//=============================
// print
//=============================
GTI_ANALYSIS_RETURN PrintSendRecv::print (int count, int rank, int sourceDest, int isSend)
{
	if (isSend)
		std::cout << "MPI_Send of " << rank << " to " << sourceDest << " with count=" << count << std::endl;
	else
		std::cout << "MPI_Recv of " << rank << " from " << sourceDest << " with count=" << count << std::endl;

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
