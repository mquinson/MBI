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
 * @file PrintKeyval.cpp
 *       @see MUST::PrintKeyval.
 *
 *  @date 12.05.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "PrintKeyval.h"

#include <assert.h>
#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(PrintKeyval);
mFREE_INSTANCE_FUNCTION(PrintKeyval);
mPNMPI_REGISTRATIONPOINT_FUNCTION(PrintKeyval);

//=============================
// Constructor
//=============================
PrintKeyval::PrintKeyval (const char* instanceName)
    : gti::ModuleBase<PrintKeyval, I_PrintKeyval> (instanceName),
      myLogger (NULL),
      myKeyvalTracker (NULL),
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
    myKeyvalTracker = (I_KeyvalTrack*) subModInstances[2];
}

//=============================
// Destructor
//=============================
PrintKeyval::~PrintKeyval ()
{
	if (myLocations)
		destroySubModuleInstance ((I_Module*) myLocations);
	myLocations = NULL;

    if (myLogger)
    		destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myKeyvalTracker)
    		destroySubModuleInstance ((I_Module*) myKeyvalTracker);
    myKeyvalTracker = NULL;
}

//=============================
// print
//=============================
GTI_ANALYSIS_RETURN PrintKeyval::print (
		MustParallelId pId,
		MustLocationId lId,
		MustKeyvalType keyval)
{
    static int index=0;
	std::stringstream stream;
	stream << "Information on key: ";
	std::list<std::pair<MustParallelId,MustLocationId> > refs;
	I_Keyval* info = myKeyvalTracker->getKeyval(pId,keyval);

	if (info)
	{
	    info->printInfo(stream, &refs);
	}
	else
	{
	    stream << "Unknown Keyvalue.";
	}

	myLogger->createMessage(index++, pId, lId, MustInformationMessage, stream.str(), refs);

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
