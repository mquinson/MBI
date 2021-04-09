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
 * @file test_cstrat_b.cxx
 *       A module that tests communication strategies and a
 *       communication protocol.
 *
 * @data 24.04.2009
 * @author Tobias Hilbrich
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <stdlib.h>
#include <string.h>
#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Module.h"
#include "ModuleBase.h"
#include "I_CommStrategyDown.h"
#include "I_CommStrategyUp.h"
#include "GtiMacros.h"
#include "GtiHelper.h"

namespace gti
{
    /**
     * Class used to drive the comm strat & comm protocol
     * testing.
     */
    class TestCStratB : public ModuleBase<TestCStratB, I_Module>, GtiHelper
    {
    protected:
        I_CommStrategyUp *strat;

    public:
        TestCStratB(const char*);
        ~TestCStratB(void);
        void run (void); /**< Runs the tests. */
    };
}

using namespace gti;

GTI_RETURN my_free_function (void* free_data, uint64_t num_bytes, void* buf)
{
    delete [] ((char*)buf);
    return GTI_SUCCESS;
}

mGET_INSTANCE_FUNCTION(TestCStratB)
mFREE_INSTANCE_FUNCTION(TestCStratB)
mPNMPI_REGISTRATIONPOINT_FUNCTION(TestCStratB)

extern "C" int MPI_Init (int* argc, char*** argv)
{
    //init MPI first
    int err = PMPI_Init (argc, argv);

    //Create the place with the given module configuration
    TestCStratB* place;
    place = TestCStratB::getInstance("");
    assert(place);
    place->run();

    return err;
}

extern "C" int MPI_Finalize (void)
{
    TestCStratB* place;
    place = TestCStratB::getInstance("");
    TestCStratB::freeInstanceForced(place);

    return PMPI_Finalize ();
}

TestCStratB::TestCStratB (const char* instanceName)
    : ModuleBase<TestCStratB, I_Module> (instanceName)
{
    //add the id to all sub modules
    char temp[64];
    sprintf (temp,"%d",buildLayer());
    addDataToSubmodules ("id", temp);

    //create sub-modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //save sub modules
    assert (subModInstances.size() == 1);
    strat = (I_CommStrategyUp*) subModInstances[0];
}

TestCStratB::~TestCStratB (void)
{
	if (strat)
	{
		strat->shutdown(GTI_FLUSH, GTI_SYNC);
		destroySubModuleInstance ((I_Module*)strat);
		strat = NULL;
	}
}

void TestCStratB::run (void)
{
    uint64_t   size;
    void            *buf,
                    *free_data;
    int             flag,
                    i,
                    rank;
    char            *text;
    GTI_RETURN (*free_function) (void* free_data, uint64_t num_bytes, void* buf);

    text = new char [256];

    // (1) Receive something from T
    strat->wait(&size, &buf, &free_data,&free_function);
    printf("Got a message: {size=%ld, content=\"%s\"}\n",size,(char*)buf);
    free_function (free_data, size, buf);

    // (2) Test till something arrives from T
    i = 0;
    do
    {
        i++;
        strat->test(&flag,&size,&buf,&free_data,&free_function);
    } while (!flag);
    printf("Got a message after %d tests: {size=%ld, content=\"%s\"}\n",i,size,(char*)buf);
    free_function (free_data, size, buf);

    // (3) Send something to the master
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    sprintf (text, "Hallo3 von %d nach oben!",rank);
    strat->send(text, strlen(text)+1, NULL, my_free_function);
}

/*EOF*/
