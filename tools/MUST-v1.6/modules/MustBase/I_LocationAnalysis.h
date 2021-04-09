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
 * @file I_LocationAnalysis.h
 *       Interface for the location module.
 *
 *  @date 04.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "mustConfig.h"
#include "LocationInfo.h"

#include "BaseIds.h"

#include <list>

#ifndef LOCATIONANALYSIS_H
#define LOCATIONANALYSIS_H

/**
 * Location analysis interface.
 */
class I_LocationAnalysis : public gti::I_Module
{
public:
	/**
	 * Registers a new location.
	 * @param pId parallel Id for the location
	 * @param lId location id to add.
	 * @param callName for the new location.
	 * @param callNameLen length of the call name.
	 * @param numStackLevels  Number of stack levels associated with this location
	 * @param stackInfosLength Total length of char array for all entries of stack levels
	 * @param indicesLength =numStackLevels*3; used for convenience
	 * @param infoIndices Array of size 3*numStackLevels with indices to the individual information pieces, for each level infoIndices[level*3+0]=symName, infoIndices[level*3+1]=lineOffset, infoIndices[level*3+2]=fileModule
	 * @param stackInfos All information pieces of all stack levels concatenated, of length stackInfosLength
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN registerLocation (
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
            ) = 0;

    /**
     * Returns information on the given id.
     * @param id to get location information for.
     * @param pId parallel id of the location.
     * @return location information for this id.
     */
    virtual must::LocationInfo & getInfoForId (MustParallelId pId, MustLocationId id) = 0;

    /**
     * Returns information on the occurrence count that the location ID decodes.
     * @param id to get occurrence count for.
     * @return location information for this id.
     */
    virtual int getOccurenceCount (MustLocationId id) = 0;

    /**
     * Creates a textual representation of the given location id.
     * @param id to get location information for.
     * @param pId parallel id of the location.
     * @return string.
     */
    virtual std::string toString (MustParallelId pId, MustLocationId id) = 0;

    /**
     * Passes information on the given pid,lid pair to the given place on the
     * same level. Uses caching to not send location information that is already
     * available on the receiver side.
     * @param pId context for lId.
     * @param lId of the location.
     * @param toPlaceId id of the place on this level to send to, must be disting from
     *              this place itself.
     * @return GTI_SUCCESS if successful, reasons for this call to not succeed may
     *              be missing intra layer communication.
     */
    virtual gti::GTI_RETURN passLocationToPlace (MustParallelId pId, MustLocationId lId, int toPlaceId) = 0;

}; /*class I_LocationAnalysis*/

#endif /*LOCATIONANALYSIS_H*/
