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
 * @file PanicReceiver.cpp
 *       @see MUST::PanicReceiver.
 *
 *  @date 11.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"

#include "PanicReceiver.h"
#include "I_CommStrategyUp.h"
#include "I_CommStrategyIntra.h"

using namespace gti;

mGET_INSTANCE_FUNCTION(PanicReceiver);
mFREE_INSTANCE_FUNCTION(PanicReceiver);
mPNMPI_REGISTRATIONPOINT_FUNCTION(PanicReceiver);

//=============================
// Constructor
//=============================
PanicReceiver::PanicReceiver (const char* instanceName)
    : gti::ModuleBase<PanicReceiver, I_PanicReceiver> (instanceName),
      myListeners ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //get module data
    bool hasIntra = false;
    std::map<std::string, std::string> data = getData();
    if (data.find("gti_layer_has_intra_comm") != data.end())
        if (data["gti_layer_has_intra_comm"].compare("1") == 0)
            hasIntra = true;

    //handle sub modules
    //first modules are all up strategies
    int i = 0;
    for (; i+1 < subModInstances.size(); i++)
        myListeners.push_back((I_PanicListener*)(I_CommStrategyUp*) subModInstances[i]);

    //last modle is either a up or a intra strategy
    if (!subModInstances.empty())
    {
        if (hasIntra)
        {
            myListeners.push_back((I_PanicListener*)(I_CommStrategyIntra*) subModInstances[i]);
        }
        else
        {
            myListeners.push_back((I_PanicListener*)(I_CommStrategyUp*) subModInstances[i]);
        }
    }

    //Initialize module data
    /*nothing to do*/
}

//=============================
// Destructor
//=============================
PanicReceiver::~PanicReceiver ()
{
    std::list<I_PanicListener*>::iterator iter;
    for (iter = myListeners.begin(); iter != myListeners.end(); iter++)
    {
        if (*iter)
            destroySubModuleInstance((I_Module*) *iter);
    }
    myListeners.clear();
}

//=============================
// notifyPanic
//=============================
GTI_ANALYSIS_RETURN PanicReceiver::notifyPanic ()
{
#ifdef GTI_DEBUG
    std::cout << getpid () << " GOT A PANIC!" << std::endl;
#endif /*GTI_DEBUG*/

    std::list<I_PanicListener*>::iterator iter;
    for (iter = myListeners.begin(); iter != myListeners.end(); iter++)
    {
        if (*iter)
            (*iter)->flushAndSetImmediate();
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// notifyFlush
//=============================
GTI_ANALYSIS_RETURN PanicReceiver::notifyFlush ()
{
#ifdef GTI_DEBUG
    std::cout << getpid () << " GOT A FLUSH!" << std::endl;
#endif /*GTI_DEBUG*/

    std::list<I_PanicListener*>::iterator iter;
    for (iter = myListeners.begin(); iter != myListeners.end(); iter++)
    {
        if (*iter)
            (*iter)->flush();
    }

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// notifyRaisePanic
//=============================
GTI_ANALYSIS_RETURN PanicReceiver::notifyRaisePanic ()
{
#ifdef GTI_DEBUG
    std::cout << getpid () << " GOT A RaisePanic!" << std::endl;
#endif /*GTI_DEBUG*/

    std::list<I_PanicListener*>::iterator iter;
    for (iter = myListeners.begin(); iter != myListeners.end(); iter++)
    {
        if (*iter)
            (*iter)->flushAndSetImmediate();
    }

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
