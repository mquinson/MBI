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
 * @file TestLog.cpp
 *       @see MUST::TestLog.
 *
 *  @date 20.01.2010
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "ModuleBase.h"
#include "I_CreateMessage.h"

#include "I_TestLog.h"

using namespace gti;

namespace must
{
	/**
     * Prints a location and parallel id to std::cout.
     */
    class TestLog : public gti::ModuleBase<TestLog, I_TestLog>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		TestLog (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		~TestLog ();

    		/**
    		 * @see I_TestLog::test
    		 */
    		GTI_ANALYSIS_RETURN test (MustParallelId pId, MustLocationId lId);

    protected:
    		I_CreateMessage *myLogCreator;
    }; /*class TestLog */
} /*namespace MUST*/

using namespace must;

mGET_INSTANCE_FUNCTION(TestLog);
mFREE_INSTANCE_FUNCTION(TestLog);
mPNMPI_REGISTRATIONPOINT_FUNCTION(TestLog);

//=============================
// Constructor
//=============================
TestLog::TestLog (const char* instanceName)
    : gti::ModuleBase<TestLog, I_TestLog> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    assert (subModInstances.size() >= 1);

    myLogCreator = (I_CreateMessage*) subModInstances[0];
}

//=============================
// Destructor
//=============================
TestLog::~TestLog ()
{
	destroySubModuleInstance ((I_Module*) myLogCreator);
}

//=============================
// print
//=============================
GTI_ANALYSIS_RETURN TestLog::test (
	MustParallelId pId,
	MustLocationId lId
	)
{

	std::list<std::pair<MustParallelId, MustLocationId> > refs;
	refs.push_back(std::make_pair (pId, lId));
	refs.push_back(std::make_pair (pId, lId));
	myLogCreator->createMessage(
			666,
			pId,
			lId,
			MustInformationMessage,
			"Test log message!",
			refs
			);

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
