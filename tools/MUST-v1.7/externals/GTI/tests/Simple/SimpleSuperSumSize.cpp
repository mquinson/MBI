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
 * @file SimpleSuperSumSize.cpp
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
    class SuperSumSize : public ModuleBase<SuperSumSize, I_SuperSumSize>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    	   SuperSumSize (const char* instanceName);

        /**
         * Performs an analysis.
         */
    	   GTI_ANALYSIS_RETURN analyse (int size);

    }; /*class SuperSumSize*/
} /*namespace gti*/

using namespace gti;

mGET_INSTANCE_FUNCTION(SuperSumSize)
mFREE_INSTANCE_FUNCTION(SuperSumSize)
mPNMPI_REGISTRATIONPOINT_FUNCTION(SuperSumSize)

SuperSumSize::SuperSumSize (const char* instanceName)
    : ModuleBase<SuperSumSize, I_SuperSumSize> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Needs no sub-modules
    assert (subModInstances.empty());
}

GTI_ANALYSIS_RETURN SuperSumSize::analyse (int size)
{
    std::cout << "SuperSumSize (size=" << size << ")" << std::endl;

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
