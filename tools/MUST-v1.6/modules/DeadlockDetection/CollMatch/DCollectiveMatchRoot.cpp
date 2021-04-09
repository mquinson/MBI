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
 * @file DCollectiveMatchRoot.cpp
 *       @see must::DCollectiveMatchRoot.
 *
 *  @date 25.04.2012
 *  @author Fabian Haensel, Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"

#include "DCollectiveMatchRoot.h"

using namespace must;

mGET_INSTANCE_FUNCTION(DCollectiveMatchRoot)
mFREE_INSTANCE_FUNCTION(DCollectiveMatchRoot)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DCollectiveMatchRoot)

//=============================
// Constructor
//=============================
DCollectiveMatchRoot::DCollectiveMatchRoot(const char* instanceName)
: DCollectiveMatch<DCollectiveMatchRoot, I_DCollectiveMatchRoot> (instanceName, false)
  {
    //Nothing to do
  }

//=============================
// Destructor
//=============================
DCollectiveMatchRoot::~DCollectiveMatchRoot()
{
    //Nothing to do
}

/*EOF*/
