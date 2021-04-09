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
 * @file BreakEnforcer.cpp
 *       @see MUST::BreakEnforcer.
 *
 *  @date 02.09.2013
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "GtiApi.h"

#include "BreakEnforcer.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(BreakEnforcer);
mFREE_INSTANCE_FUNCTION(BreakEnforcer);
mPNMPI_REGISTRATIONPOINT_FUNCTION(BreakEnforcer);

//=============================
// Constructor
//=============================
BreakEnforcer::BreakEnforcer (const char* instanceName)
    : gti::ModuleBase<BreakEnforcer, I_BreakEnforcer> (instanceName),
      myStrats (),
      mySecLastTest (0)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    for (int i = 0; i < subModInstances.size(); i++)
        myStrats.push_back((I_CommStrategyUp*) subModInstances[i]);

    //Initialize module data
    /*nothing to do*/
}

//=============================
// Destructor
//=============================
BreakEnforcer::~BreakEnforcer ()
{
    std::list<I_CommStrategyUp*>::iterator iter;
    for (iter = myStrats.begin(); iter != myStrats.end(); iter++)
    {
        if (*iter)
            destroySubModuleInstance ((I_Module*) *iter);
    }
    myStrats.clear();
}

//=============================
// test
//=============================
GTI_ANALYSIS_RETURN BreakEnforcer::test (void)
{
    GTI_ANALYSIS_RETURN ret = GTI_ANALYSIS_FAILURE;
    struct timeval currentTime;
    uint64_t currentSec;
    gettimeofday(&currentTime, NULL);
    currentSec = currentTime.tv_sec;

    if(currentSec <= mySecLastTest)
        return GTI_ANALYSIS_SUCCESS;

    //Enough time passed we test for incoming control messages
    mySecLastTest = currentSec;

    std::list<I_CommStrategyUp*>::iterator iter;
    for (iter = myStrats.begin(); iter != myStrats.end(); iter++)
    {
        I_CommStrategyUp* s = *iter;
        int code = 0;

        do
        {
            int success = 0;
            uint64_t size;
            void* buf;
            void* freeData;
            GTI_RETURN (*freeFunction) (void* free_data, uint64_t num_bytes, void* buf);

            if (s->test (
                    &success,
                    &size,
                    &buf,
                    &freeData,
                    &freeFunction
            ) == GTI_SUCCESS)
            {
                if (success)
                {
                    //First 8 bytes are user defined
                    /**
                     * @todo this is non-portable and uses implicit knowledge from MUST's
                     * defauls trace record generator implementation.
                     */
                    int *pCode = (int*)(((char*)buf)+8);
                    code = *pCode;

                    //Code must be 0 or 1, we use no other values
                    assert (code == 0 || code == 1);

                    //debug output
                    /*
                    if (code == 1)
                    {
                        std::cout << getpid() << "Got break!" << std::endl;
                    }
                    else if (code == 0)
                    {
                        std::cout << getpid() << "Got Continue!" << std::endl;
                    }
                    */

                    //Free event memory
                    (*freeFunction) (freeData, size, buf);
                }
            }

            //Add some sleep to avoid heavy busy waiting
            if (code && !success)
                usleep (10);
        } while (code != 0);
    }

    return ret;
}

//=============================
// handleBroadcastBreak
//=============================
GTI_ANALYSIS_RETURN BreakEnforcer::handleBroadcastBreak (
        int code)
{
    /**
     * Dummy should never be called.
     * The actual handling happens in BreakEnforcer::test currently.
     */
    assert(0);
    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
