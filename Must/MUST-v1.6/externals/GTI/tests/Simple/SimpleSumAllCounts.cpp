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
 * @file SimpleSumAllCounts.cpp
 *       A module for the simple weaver test case.
 *
 *  @date 25.08.2010
 *  @author Tobias Hilbrich
 */

#include <assert.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <iostream>
#include "GtiMacros.h"
#include "SimpleMod.h"
#include "ModuleBase.h"
#include "GtiEnums.h"

namespace gti
{
	/**
     * Check class.
     */
    class SumAllCounts : public ModuleBase<SumAllCounts, I_SumAllCounts>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    	    SumAllCounts (const char* instanceName);

        /**
         * Performs an analysis.
         */
    	    GTI_ANALYSIS_RETURN analyse (int count);

    }; /*class SumAllCounts*/
} /*namespace gti*/

using namespace gti;

mGET_INSTANCE_FUNCTION(SumAllCounts)
mFREE_INSTANCE_FUNCTION(SumAllCounts)
mPNMPI_REGISTRATIONPOINT_FUNCTION(SumAllCounts)

SumAllCounts::SumAllCounts (const char* instanceName)
    : ModuleBase<SumAllCounts, I_SumAllCounts> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Needs no sub-modules
    assert (subModInstances.empty());
}

GTI_ANALYSIS_RETURN SumAllCounts::analyse (int count)
{
    std::cout << "SumAllCounts (count=" << count << ")" << std::endl;

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
