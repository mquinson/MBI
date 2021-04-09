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
 * @file SimpleCheckArray.cpp
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
    class CheckArray : public ModuleBase<CheckArray, I_CheckArray>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    	CheckArray (const char* instanceName);

        /**
         * Performs an analysis.
         */
    	GTI_ANALYSIS_RETURN analyse (int, int*);

    }; /*class CheckArray*/
} /*namespace gti*/

using namespace gti;

mGET_INSTANCE_FUNCTION(CheckArray)
mFREE_INSTANCE_FUNCTION(CheckArray)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CheckArray)

CheckArray::CheckArray (const char* instanceName)
    : ModuleBase<CheckArray, I_CheckArray> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Needs no sub-modules
    assert (subModInstances.empty());
}

GTI_ANALYSIS_RETURN CheckArray::analyse (int c, int* array)
{
    std::cout << "CheckArray (c=" << c << ", array={";

    for (int i = 0; i < c; i++)
    {
    	if (i != 0) std::cout << ", ";
    	std::cout << array[i];
    }

    std::cout << "})" << std::endl;

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
