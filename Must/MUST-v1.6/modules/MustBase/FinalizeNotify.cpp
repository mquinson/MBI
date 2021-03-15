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
 * @file FinalizeNotify.cpp
 *       @see MUST::FinalizeNotify.
 *
 *  @date 04.04.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "MustEnums.h"
#include "BaseApi.h"

#include "FinalizeNotify.h"

using namespace must;

mGET_INSTANCE_FUNCTION(FinalizeNotify)
mFREE_INSTANCE_FUNCTION(FinalizeNotify)
mPNMPI_REGISTRATIONPOINT_FUNCTION(FinalizeNotify)

//=============================
// Constructor
//=============================
FinalizeNotify::FinalizeNotify (const char* instanceName)
    : gti::ModuleBase<FinalizeNotify, I_FinalizeNotify> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

#define NUM_SUBS 0
    //handle sub modules
#if NUM_SUBS>0
    if (subModInstances.size() < NUM_SUBS)
    {
    		std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
    		assert (0);
    }
#endif
    if (subModInstances.size() > NUM_SUBS)
    {
    		for (unsigned int i = NUM_SUBS; i < subModInstances.size(); i++)
    			destroySubModuleInstance (subModInstances[i]);
    }
}

//=============================
// Destructor
//=============================
FinalizeNotify::~FinalizeNotify ()
{
	/*Nothing to do*/
}

//=============================
// notify
//=============================
GTI_ANALYSIS_RETURN FinalizeNotify::notify ()
{
	finalizeNotifyP fP;
	if (getWrapperFunction ("finalizeNotify", (GTI_Fct_t*)&fP) == GTI_SUCCESS)
	{
		(*fP) ();
	}
	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
