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
 * @file Template.cpp
 *       @see MUST::Template.
 *
 *  @date 21.01.2011
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"

#include "Template.h"

using namespace must;

mGET_INSTANCE_FUNCTION(Template);
mFREE_INSTANCE_FUNCTION(Template);
mPNMPI_REGISTRATIONPOINT_FUNCTION(Template);

//=============================
// Constructor
//=============================
Template::Template (const char* instanceName)
    : gti::ModuleBase<Template, I_Template> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
    //TODO
}

//=============================
// Destructor
//=============================
Template::~Template ()
{
    //TODO
}

//=============================
//
//=============================
GTI_ANALYSIS_RETURN Template::analysisFunction (void)
{
	//TODO

	return GTI_ANALYSIS_SUCCESS;
}

/*EOF*/
