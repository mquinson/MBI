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
 * @file PrintDatatype.cpp
 *       @see MUST::PrintDatatype.
 *
 *  @date 23.02.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"

#include "PrintDatatype.h"
#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(PrintDatatype);
mFREE_INSTANCE_FUNCTION(PrintDatatype);
mPNMPI_REGISTRATIONPOINT_FUNCTION(PrintDatatype);

//=============================
// Constructor
//=============================
PrintDatatype::PrintDatatype (const char* instanceName)
    : gti::ModuleBase<PrintDatatype, I_PrintDatatype> (instanceName),
      myTypes(NULL), myLog(NULL)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    assert (subModInstances.size() == 2);
    myTypes = (I_DatatypeTrack*) subModInstances[0];
    myLog = (I_CreateMessage*) subModInstances[1];
}

//=============================
// Destructor
//=============================
PrintDatatype::~PrintDatatype ()
{
    if (myTypes)
    		destroySubModuleInstance ((I_Module*) myTypes);
    myTypes = NULL;

    if (myLog)
    		destroySubModuleInstance ((I_Module*) myLog);
    myLog = NULL;
}

//=============================
// print
//=============================
GTI_ANALYSIS_RETURN PrintDatatype::print (
		MustParallelId pId,
		MustLocationId lId,
		MustDatatypeType type)
{
    static int index=0;
	std::stringstream stream;
	stream << "Information on datatype: ";
	std::list<std::pair<MustParallelId,MustLocationId> > refs;
	I_Datatype* info = myTypes->getDatatype(pId, type);

	if (info)
	{
	    info->printInfo(stream, &refs);
	}
	else
	{
	    stream << "Unknown Datatype.";
	}

	myLog->createMessage(index++, pId, lId, MustInformationMessage, stream.str(), refs);

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
