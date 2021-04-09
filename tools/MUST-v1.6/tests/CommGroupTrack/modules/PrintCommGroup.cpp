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
 * @file PrintCommGroup.cpp
 *       @see MUST::PrintCommGroup.
 *
 *  @date 06.03.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "PrintCommGroup.h"

#include <assert.h>
#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(PrintCommGroup);
mFREE_INSTANCE_FUNCTION(PrintCommGroup);
mPNMPI_REGISTRATIONPOINT_FUNCTION(PrintCommGroup);

//=============================
// Constructor
//=============================
PrintCommGroup::PrintCommGroup (const char* instanceName)
    : gti::ModuleBase<PrintCommGroup, I_PrintCommGroup> (instanceName),
      myLocations (NULL),
      myLogger (NULL),
      myCTracker (NULL),
      myGTracker (NULL)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    if (subModInstances.size () < 4)
    {
    		std::cerr << "Error: " << __LINE__ << "@" << __FILE__ << " has not enough sub modules, aborting!" << std::endl;
    		assert (0);
    }
    if (subModInstances.size() > 4)
    {
    		for (std::vector<I_Module*>::size_type i = 4; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }

    myLocations = (I_LocationAnalysis*) subModInstances[0];
    myLogger = (I_CreateMessage*) subModInstances[1];
    myCTracker = (I_CommTrack*) subModInstances[2];
    myGTracker = (I_GroupTrack*) subModInstances[3];
}

//=============================
// Destructor
//=============================
PrintCommGroup::~PrintCommGroup ()
{
	if (myLocations)
		destroySubModuleInstance ((I_Module*) myLocations);
	myLocations = NULL;

    if (myLogger)
    		destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myCTracker)
    		destroySubModuleInstance ((I_Module*) myCTracker);
    myCTracker = NULL;

    if (myGTracker)
    		destroySubModuleInstance ((I_Module*) myGTracker);
    myGTracker = NULL;
}

//=============================
// printComm
//=============================
GTI_ANALYSIS_RETURN PrintCommGroup::printComm (
		MustParallelId pId,
		MustLocationId lId,
		MustCommType comm)
{
    static int index=0;
	std::stringstream stream;
	stream << "Information on communicator: ";
	std::list<std::pair<MustParallelId,MustLocationId> > refs;

	//Extra information for comm contents
	I_Comm* info = myCTracker->getComm(pId, comm);

	if (!info)
	{
	    std::cout << "Unknown Communicator." << std::endl;
	}
	else
	{
        info->printInfo(stream, &refs);

        if (info != NULL && !info->isNull())
        {
            stream << " (";
            addGroupInfoToStream (info->getGroup(), stream);

            if (info->isIntercomm())
            {
                stream << ", remote group: ";
                addGroupInfoToStream (info->getRemoteGroup(), stream);
            }

            stream << " contextId=" << info->getContextId() << ")";
        }
	}

	myLogger->createMessage(index++, pId, lId, MustInformationMessage, stream.str(), refs);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// printGroup
//=============================
GTI_ANALYSIS_RETURN PrintCommGroup::printGroup (
		MustParallelId pId,
		MustLocationId lId,
		MustGroupType group)
{
    static int index=0;
	std::stringstream stream;
	stream << "Information on group: ";
	std::list<std::pair<MustParallelId,MustLocationId> > refs;

	I_Group* info = myGTracker->getGroup(pId, group);

	if (!info)
	{
	    stream << "Unknown Group." << std::endl;
	}
	else
	{
        info->printInfo(stream, &refs);

        //==1) Invalid
        if (!info->isNull())
        {
            //Put information about group layout into stream
            stream << " (";
            addGroupInfoToStream (info->getGroup(), stream);
            stream << ")";
        }
	}

	myLogger->createMessage(index++, pId, lId, MustInformationMessage, stream.str(), refs);

	return GTI_ANALYSIS_SUCCESS;
}

//=============================
// addGroupInfoToStream
//=============================
void PrintCommGroup::addGroupInfoToStream (I_GroupTable* group, std::stringstream &stream)
{
	if (group)
	{
		stream << "size=" << group->getSize() << " table: ";
		for (int i = 0; i < group->getSize(); i++)
		{
			if (i != 0)
				stream << "; ";

			int t;
			group->translate(i,&t);
			stream << i << "->" << t;
		}
	}
	else
	{
		stream << " pointer to I_Group is NULL";
	}
}

/*EOF*/
