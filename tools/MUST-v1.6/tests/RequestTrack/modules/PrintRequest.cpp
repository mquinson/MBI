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
 * @file PrintRequest.cpp
 *       @see MUST::PrintRequest.
 *
 *  @date 04.02.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "PrintRequest.h"

#include <assert.h>
#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(PrintRequest);
mFREE_INSTANCE_FUNCTION(PrintRequest);
mPNMPI_REGISTRATIONPOINT_FUNCTION(PrintRequest);

//=============================
// Constructor
//=============================
PrintRequest::PrintRequest (const char* instanceName)
    : gti::ModuleBase<PrintRequest, I_PrintRequest> (instanceName),
      myLogger (NULL),
      myRTracker (NULL),
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
    myRTracker = (I_RequestTrack*) subModInstances[2];
}

//=============================
// Destructor
//=============================
PrintRequest::~PrintRequest ()
{
	if (myLocations)
		destroySubModuleInstance ((I_Module*) myLocations);
	myLocations = NULL;

    if (myLogger)
    		destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myRTracker)
    		destroySubModuleInstance ((I_Module*) myRTracker);
    myRTracker = NULL;
}

//=============================
// print
//=============================
GTI_ANALYSIS_RETURN PrintRequest::print (
		MustParallelId pId,
		MustLocationId lId,
		MustRequestType request)
{
    static int index=0;
	std::stringstream stream;
	stream << "Information on request: ";
	std::list<std::pair<MustParallelId,MustLocationId> > refs;
	I_Request* info = myRTracker->getRequest(pId,request);

	if (info)
	{
	    info->printInfo(stream, &refs);
	}
	else
	{
	    stream << "Unknown Request.";
	}

	myLogger->createMessage(index++, pId, lId, MustInformationMessage, stream.str(), refs);

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
