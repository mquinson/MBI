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
 * @file DCollectiveMatchReduction.cpp
 *       @see must::DCollectiveMatchReduction.
 *
 *  @date 25.04.2012
 *  @author Fabian Haensel, Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"

#include "DCollectiveMatchReduction.h"

using namespace must;

mGET_INSTANCE_FUNCTION(DCollectiveMatchReduction)
mFREE_INSTANCE_FUNCTION(DCollectiveMatchReduction)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DCollectiveMatchReduction)

//=============================
// Constructor
//=============================
DCollectiveMatchReduction::DCollectiveMatchReduction(const char* instanceName)
: DCollectiveMatch<DCollectiveMatchReduction, I_DCollectiveMatchReduction> (instanceName, true)
  {
    //Nothing to do
  }

//=============================
// Destructor
//=============================
DCollectiveMatchReduction::~DCollectiveMatchReduction()
{
    //Nothing to do
}

/*EOF*/
