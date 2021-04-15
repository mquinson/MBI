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
 * @file SimplePrintFloatSum.cpp
 *       A module for the simple weaver test case.
 *
 *  @date 01.12.2010
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
    class PrintFloatSum : public ModuleBase<PrintFloatSum, I_PrintFloatSum>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		PrintFloatSum (const char* instanceName);

        /**
         * Performs an analysis.
         */
    	    GTI_ANALYSIS_RETURN analyse (float f);

    }; /*class PrintFloatSum*/
} /*namespace gti*/

using namespace gti;

mGET_INSTANCE_FUNCTION(PrintFloatSum)
mFREE_INSTANCE_FUNCTION(PrintFloatSum)
mPNMPI_REGISTRATIONPOINT_FUNCTION(PrintFloatSum)

PrintFloatSum::PrintFloatSum (const char* instanceName)
    : ModuleBase<PrintFloatSum, I_PrintFloatSum> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Needs no sub-modules
    assert (subModInstances.empty());
}

GTI_ANALYSIS_RETURN PrintFloatSum::analyse (float f)
{
    std::cout << "PrintFloatSum (f=" << f << ")" << std::endl;

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
