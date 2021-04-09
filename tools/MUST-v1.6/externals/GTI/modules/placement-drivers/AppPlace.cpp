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
 * @file AppPlace.cpp
 *       A placement driver that is spawned as a thread in an
 *       application process.
 *
 *
 * @author Tobias Hilbrich, Felix Münchhalven, Joachim Protze
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>

#include <pnmpimod.h>
#include "AppPlace.h"
#include "GtiMacros.h"
#include "SuspensionBufferTree.h"

using namespace gti;

mGET_INSTANCE_FUNCTION (AppPlace)
mFREE_INSTANCE_FUNCTION (AppPlace)
mPNMPI_REGISTRATIONPOINT_FUNCTION(AppPlace)

//=============================
// AppPlace
//=============================
AppPlace::AppPlace (const char* instanceName)
    : ModuleBase<AppPlace, I_Place> (instanceName),
      myCommStratUp (NULL)
{
//    registerName(instanceName);
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //save sub modules
    assert (subModInstances.size() == 1);
    myCommStratUp = (I_CommStrategyUp*) subModInstances[0];
}

//=============================
// ~AppPlace
//=============================
AppPlace::~AppPlace (void)
{
    if (myCommStratUp)
    {
        destroySubModuleInstance ((I_Module*) myCommStratUp);
        myCommStratUp = NULL;
    }
}

//=============================
// getNodeInLayerId
//=============================
GTI_RETURN AppPlace::getNodeInLayerId (GtiTbonNodeInLayerId* id)
{
  return myCommStratUp->getPlaceId(id);
}

//=============================
// getLayerIdForApplicationRank
//=============================
GTI_RETURN AppPlace::getLayerIdForApplicationRank (int rank, GtiTbonNodeInLayerId* id)
{
  return GTI_ERROR;
}


/*EOF*/
