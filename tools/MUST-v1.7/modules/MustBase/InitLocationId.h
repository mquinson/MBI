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
 * @file InitLocationId.h
 *       @see must::InitLocationId.
 *
 *  @date 24.04.2014
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_InitLocationId.h"
#include "mustConfig.h"
#include "LocationInfo.h"
#include "I_InitParallelId.h"
#include "BaseApi.h"

#ifndef INITLOCATIONID_H
#define INITLOCATIONID_H

using namespace gti;

namespace must
{
	/**
     * Implementation to set the location ID with either just a callname or with a callname and a call stack.
     */
    class InitLocationId : public gti::ModuleBase<InitLocationId, I_InitLocationId>
    {
    protected:
        /**
         * Maps location id to its information.
         * Two versions depending of whether we have stack information or not.
         */
#ifndef USE_CALLPATH
        /** Maps locationID (which is just a GTI call id in that case) to both a information that describes this location and an occurrence count*/
        typedef std::map<MustLocationId, std::pair<LocationInfo, uint32_t> > KnownLocationsType;
#else
        /** Maps a GTI call id to a map of call stacks known for that call id and an occurrence count*/
        typedef std::map<int, std::pair<std::map<LocationInfo, MustLocationId>, uint32_t> > KnownLocationsType;
#endif

        KnownLocationsType myKnownLocations;

        I_InitParallelId *myPIdInit;
        handleNewLocationP myNewLocFct;

#ifdef USE_CALLPATH
        /**
         * Helper function to call the handleNewLocation call when we use the callpath module
         * @param id of the new location
         * @param callName of the call
         * @param location stack information
         */
        void createHandleNewLocationCall (MustLocationId id, char* callName, LocationInfo &location);
#endif


    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
        InitLocationId (const char* instanceName);

        /**
         * @see I_InitLocationId::init
         */
         GTI_ANALYSIS_RETURN init (MustLocationId *pStorage, const char* callName, int callId);

    }; /*class InitLocationId */
} /*namespace MUST*/

#endif /*INITLOCATIONID_H*/
