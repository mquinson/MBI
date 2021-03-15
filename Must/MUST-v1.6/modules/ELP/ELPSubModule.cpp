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
 * @file ELPSubModule.cpp
 *       @see MUST::ELPSubModule.
 *
 *  @date 22.07.2014
 *  @author Felix Muenchhalfen
 */

#include "GtiMacros.h"
#include "ELPSubModule.h"
#include "MustEnums.h"

using namespace must;

mGET_INSTANCE_FUNCTION(ELPSubModule)
mFREE_INSTANCE_FUNCTION(ELPSubModule)
mPNMPI_REGISTRATIONPOINT_FUNCTION(ELPSubModule)

ELPSubModule::ELPSubModule(const char* instanceName) : gti::ModuleBase<ELPSubModule, I_ELPSubModule> (instanceName) 
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODULES 0
    if (subModInstances.size() < NUM_MODULES)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
    if (subModInstances.size() > NUM_MODULES)
    {
    		for (int i = NUM_MODULES; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }

    //myLogger = (I_CreateMessage*)subModInstances[0];

    //Initialize module data
    //Nothing to do
}

ELPSubModule::~ELPSubModule() {
    /*if( myLogger != NULL )
        destroySubModuleInstance( myLogger );
    myLogger = NULL;*/
}

