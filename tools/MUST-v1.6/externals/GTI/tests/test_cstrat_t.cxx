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
 * @file test_cstrat_t.cxx
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
    class TestCStratT : public ModuleBase<TestCStratT, I_Module>, GtiHelper
    {
    protected:
        I_CommStrategyDown *strat;

    public:
        TestCStratT(const char*);
        ~TestCStratT(void);
        void run (void); /**< Runs the tests. */
    };
}

using namespace gti;

GTI_RETURN my_free_function (void* free_data, uint64_t num_bytes, void* buf)
{
    delete [] ((char*)buf);
    return GTI_SUCCESS;
}

mGET_INSTANCE_FUNCTION(TestCStratT)
mFREE_INSTANCE_FUNCTION(TestCStratT)
mPNMPI_REGISTRATIONPOINT_FUNCTION(TestCStratT)

extern "C" int MPI_Init (int* argc, char*** argv)
{
    //init MPI first
    int err = PMPI_Init (argc, argv);

    //Create the place with the given module configuration
    TestCStratT* place;
    place = TestCStratT::getInstance("");
    assert(place);
    place->run();

    return err;
}

extern "C" int MPI_Finalize (void)
{
    TestCStratT* place;
    place = TestCStratT::getInstance("");
    TestCStratT::freeInstanceForced(place);

    return PMPI_Finalize ();
}

TestCStratT::TestCStratT (const char* instanceName)
    : ModuleBase<TestCStratT, I_Module> (instanceName)
{
    //add the id to all sub modules
    char temp[64];
    sprintf (temp,"%ld",buildLayer());
    addDataToSubmodules ("id", temp);

    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //save sub modules
    assert (subModInstances.size() == 1);
    strat = (I_CommStrategyDown*) subModInstances[0];

}

TestCStratT::~TestCStratT (void)
{
	if (strat)
	{
		strat->shutdown(GTI_FLUSH, GTI_SYNC);
		destroySubModuleInstance ((I_Module*)strat);
		strat = NULL;
	}
}

void TestCStratT::run (void)
{
    uint64_t   size,
                    numClients,
                    i,
                    iters;
    int             flag;
    char            *text;
    void            *buf,
                    *free_data;
    uint64_t fromChannel;
    GTI_RETURN (*free_function) (void* free_data, uint64_t num_bytes, void* buf);


    text = new char[256];

    //(1) Send to B
    sprintf (text, "Hallo1 !");
    strat->broadcast(text, strlen(text)+1, NULL, NULL);

    //(2) Send to B
    sprintf (text, "Hallo2 !");
    strat->broadcast(text, strlen(text)+1, NULL, my_free_function);

    //(3) Receive something from all guys
    strat->getNumClients(&numClients);
    i = 0;
    iters = 1;
    while (i < numClients)
    {
        if (i % 2)
        {
            strat->wait(&size, &buf, &free_data,&free_function, &fromChannel);
            printf("Got a message via wait: {size=%ld, content=\"%s\"}\n",size,(char*)buf);
            free_function (free_data, size, buf);
            i++;
            strat->acknowledge(fromChannel);
        }
        else
        {
            strat->test(&flag, &size, &buf, &free_data, &free_function, &fromChannel);

            if (flag)
            {
                printf("Got a message after %ld tests: {size=%ld, content=\"%s\"}\n",iters,size,(char*)buf);
                free_function (free_data, size, buf);
                iters = 0;
                i++;
                strat->acknowledge(fromChannel);
            }

            iters++;
        }
    }//for num channels

}

/*EOF*/
