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
 * @file PrintFloatSum.cpp
 *       Implementation for a print module of a reduction example.
 *
 *  @date 23.12.2010
 *  @author Tobias Hilbrich
 */

#include <iostream>
#include "GtiMacros.h"
#include "ModuleBase.h"
#include "PrintFloatSum.h"

namespace gti
{
	/**
     * Module for printing sums.
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
         * Performs the printing.
         */
    	    GTI_ANALYSIS_RETURN print (float f, I_ChannelId* id);
    }; /*class PrintFloatSum*/
} /*namespace gti*/

using namespace gti;

mGET_INSTANCE_FUNCTION(PrintFloatSum)
mFREE_INSTANCE_FUNCTION(PrintFloatSum)
mPNMPI_REGISTRATIONPOINT_FUNCTION(PrintFloatSum)

//=============================
// PrintFloatSum
//=============================
PrintFloatSum::PrintFloatSum (const char* instanceName)
    : ModuleBase<PrintFloatSum, I_PrintFloatSum> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();
    //No sub modules needed ...
}

//=============================
// print
//=============================
GTI_ANALYSIS_RETURN PrintFloatSum::print (float f, I_ChannelId* id)
{
	std::string channStr = "CompletlyReduced";

	if (id)
		channStr = id->toString();

    std::cout << "PrintFloatSum (f=" << f << ") ChannelId=" << channStr << std::endl;
    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
