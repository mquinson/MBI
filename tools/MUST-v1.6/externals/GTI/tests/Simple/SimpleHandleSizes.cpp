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
 * @file SimpleHandleSizes.cpp
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
    class HandleSizes : public ModuleBase<HandleSizes, I_HandleSizes>
    {
    protected:

    public:
			/**
			 * Constructor.
			 * @param instanceName name of this module instance.
			 */
			HandleSizes (const char* instanceName);

			/**
			 * Destructor.
			 */
			virtual ~HandleSizes (void);

			/**
			 * Performs an analysis.
			 */
			GTI_ANALYSIS_RETURN analyse (int,int*);

			/**
			 * Performs another analysis.
			 */
			GTI_ANALYSIS_RETURN analyse2 (int);

    protected:
			I_Module* mySubMod;
    }; /*class HandleSizes*/
} /*namespace gti*/

using namespace gti;

mGET_INSTANCE_FUNCTION(HandleSizes)
mFREE_INSTANCE_FUNCTION(HandleSizes)
mPNMPI_REGISTRATIONPOINT_FUNCTION(HandleSizes)

HandleSizes::HandleSizes (const char* instanceName)
    : ModuleBase<HandleSizes, I_HandleSizes> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Needs no sub-modules
    assert (subModInstances.size() == 1);

    mySubMod = subModInstances[0];
}

HandleSizes::~HandleSizes (void)
{
	if (mySubMod)
		destroySubModuleInstance ((I_Module*) mySubMod);
	mySubMod = NULL;
}

GTI_ANALYSIS_RETURN HandleSizes::analyse (int c, int* array)
{
    std::cout << "HandleSizes:analyse (c=" << c << ", array={";

    for (int i = 0; i < c; i++)
    {
    	if (i != 0) std::cout << ", ";
    	std::cout << array[i];
    }

    std::cout << "})" << std::endl;

    //Call new size ...
    int (*newSize) (int);
    if (getWrapperFunction ("newSize", (GTI_Fct_t*)&newSize) == GTI_SUCCESS)
    {
    	for (int i = 0; i < c; i++)
    		(*newSize) (array[i]);
    }
    else
    {
    	std::cout << "ERROR: failed to get \"newSize\" function pointer from wrapper." << std::endl;
    }

    return GTI_ANALYSIS_SUCCESS;
}

GTI_ANALYSIS_RETURN HandleSizes::analyse2 (int c)
{
    std::cout << "HandleSizes:analyse2 (c=" << c << ")" << std::endl;

    return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/

