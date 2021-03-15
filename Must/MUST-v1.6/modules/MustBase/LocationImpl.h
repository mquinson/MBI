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
 * @file LocationImpl.h
 *       Implementation for the location id analysis interface.
 *
 *  @date 07.01.2010
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "BaseApi.h"
#include "I_LocationAnalysis.h"

#include <set>

#ifndef LOCATIONIMPL_H
#define LOCATIONIMPL_H

using namespace gti;

namespace must
{
	/**
     * Implementation for the location id analysis.
     *
     * Splits location IDs into two parts (32 bit, 32bit), upper 32 bits are an occurence count that can directly be decoded,
     * the lower 32bit are used to identify additional information for the call location as below:
     *
     * Uses two maps, in order to try to provide a simple and reducable mapping
     * for the location ids.
     * The global map is used for the first occurrence of a location id and has no parallel id.
     * A local map is used for conflicting occurrences of the same location id and uses a
     * parallel id in addition.
     *
     * When it registers an id it tries the following:
     * 1) If this location id is already in the global map with the same location information -> done
     * 2) If no entry for this id is in the global map -> add it to global map
     * 3) If another entry is in the global map -> add it to local map with given parallel id
     *
     * When looking up information on a location ID, it tries the following:
     * 1) If there is an entry in the local map for the location and parallel id pair -> return the information from that one
     * 2) Else return the information in the global map
     */
    class LocationImpl : public gti::ModuleBase<LocationImpl, I_LocationAnalysis>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		LocationImpl (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~LocationImpl ();

    		/**
    		 * @see I_LocationAnalysis::registerLocation
    		 */
    		GTI_ANALYSIS_RETURN registerLocation (
    		        MustParallelId pId,
    		        MustLocationId lId,
    		        const char* callName,
    		        int callNameLen
#ifdef USE_CALLPATH
    		        ,
    		        int numStackLevels,
    		        int stackInfosLength,
    		        int indicesLength,
    		        int* infoIndices,
    		        char* stackInfos
#endif
            );

        /**
         * @see I_LocationAnalysis::getInfoForId.
         */
    		LocationInfo& getInfoForId (MustParallelId pId, MustLocationId lId);

    		/**
    		 * @see I_LocationAnalysis::getOccurenceCount.
    		 */
    		int getOccurenceCount (MustLocationId id);

    		/**
    		  * @see I_LocationAnalysis::toString
    		  */
    		std::string toString (MustParallelId pId, MustLocationId id);

    		/**
    		 * @see I_LocationAnalysis::passLocationToPlace
    		 */
    		GTI_RETURN passLocationToPlace (MustParallelId pId, MustLocationId lId, int toPlaceId);

    protected:
    		typedef std::map<std::pair<MustParallelId, MustLocationId>, LocationInfo> LocalMapType;
    		typedef std::map<MustLocationId, LocationInfo> GlobalMapType;

    		GlobalMapType myGlobalMap; /**< Used if no local mapping of a location id is present.*/
    		LocalMapType myLocalMap; /**< Mapping for an individual parallel element and an location id to the location information (local mapping).*/

    		typedef std::pair<std::pair<MustParallelId,MustLocationId>, int > ForwardKey;
    		typedef std::set<ForwardKey> ForwardType;
    		ForwardType myForwardedIds;
    		passLocationAcrossP myPassAcross; /**< Function pointer to cal for passing a location to another place on the same level.*/

    		LocationInfo myEmptyInfo;
    }; /*class LocationImpl */
} /*namespace MUST*/

#endif /*LOCATIONIMPL_H*/
