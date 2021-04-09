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
 * @file ThreadedAppStartup.h
 *       Header for a tool thread spawning driver
 *
 * @author Felix Muenchhalfen
 *
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Module.h"
#include "GtiHelper.h"
#include "ModuleBase.h"
#include "I_CommStrategyUp.h"
#include "I_CommStrategyDown.h"
#include "I_CommStrategyIntra.h"
#include "I_PlaceReceival.h"
#include "I_Profiler.h"
#include "I_FloodControl.h"

#ifndef THREADED_APP_STARTUP_H
#define THREADED_APP_STARTUP_H

void *threadProc( void *thread_data );

#endif /* THREADED_APP_STARTUP_H */
