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
 * @file PrintOp.cpp
 *       @see MUST::PrintOp.
 *
 *  @date 10.05.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "PrintOp.h"

#include <assert.h>
#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(PrintOp);
mFREE_INSTANCE_FUNCTION(PrintOp);
mPNMPI_REGISTRATIONPOINT_FUNCTION(PrintOp);

//=============================
// Constructor
//=============================
PrintOp::PrintOp (const char* instanceName)
    : gti::ModuleBase<PrintOp, I_PrintOp> (instanceName),
      myLogger (NULL),
      myOpTracker (NULL),
      myLocations (NULL)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    if (subModInstances.size () < 3)
    {
    		std::cerr << "Error: " << __LINE__ << "@" << __FILE__ << " has not enough sub modules, aborting!" << std::endl;
    		assert (0);
    }

    myLocations = (I_LocationAnalysis*) subModInstances[0];
    myLogger = (I_CreateMessage*) subModInstances[1];
    myOpTracker = (I_OpTrack*) subModInstances[2];
}

//=============================
// Destructor
//=============================
PrintOp::~PrintOp ()
{
	if (myLocations)
		destroySubModuleInstance ((I_Module*) myLocations);
	myLocations = NULL;

    if (myLogger)
    		destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myOpTracker)
    		destroySubModuleInstance ((I_Module*) myOpTracker);
    myOpTracker = NULL;
}

//=============================
// print
//=============================
GTI_ANALYSIS_RETURN PrintOp::print (
		MustParallelId pId,
		MustLocationId lId,
		MustOpType op)
{
    static int index=0;
	std::stringstream stream;
	stream << "Information on operation: ";
	std::list<std::pair<MustParallelId,MustLocationId> > refs;
	I_Op* info = myOpTracker->getOp(pId,op);

	if (info)
	{
	    info->printInfo(stream, &refs);
	}
	else
	{
	    stream << "Unknown Operation.";
	}

	myLogger->createMessage(index++, pId, lId, MustInformationMessage, stream.str(), refs);

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
