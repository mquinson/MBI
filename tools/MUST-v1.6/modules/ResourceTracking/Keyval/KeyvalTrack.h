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
 * @file KeyvalTrack.h
 *       @see MUST::KeyvalTrack
 *
 *  @date 12.05.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "TrackBase.h"

#include "I_KeyvalTrack.h"
#include "Keyval.h"

#include <map>

#ifndef KEYVALTRACK_H
#define KEYVALTRACK_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_KeyvalTrack.
     */
    class KeyvalTrack : public TrackBase<Keyval, I_Keyval, MustKeyvalType, MustMpiKeyvalPredefined, KeyvalTrack, I_KeyvalTrack>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		KeyvalTrack (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~KeyvalTrack (void);

    		/**
    		 * @see I_KeyvalTrack::keyvalCreate
    		 */
    		GTI_ANALYSIS_RETURN keyvalCreate (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustKeyvalType newKeyval);

    		/**
    		 * @see I_KeyvalTrack::keyvalFree
    		 */
    		GTI_ANALYSIS_RETURN keyvalFree (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustKeyvalType keyval);

    		/**
    		 * @see I_KeyvalTrack::getKeyval
    		 */
    		I_Keyval* getKeyval (
    				MustParallelId pId,
    				MustKeyvalType keyval);

    		/**
    		 * @see I_KeyvalTrack::getKeyval
    		 */
    		I_Keyval* getKeyval (
    				int rank,
    				MustKeyvalType keyval);

    		/**
    		 * @see I_KeyvalTrack::getPersistentKeyval
    		 */
    		I_KeyvalPersistent* getPersistentKeyval (
    		        MustParallelId pId,
    		        MustKeyvalType keyval);

    		/**
    		 * @see I_KeyvalTrack::getPersistentKeyval
    		 */
    		I_KeyvalPersistent* getPersistentKeyval (
    		        int rank,
    		        MustKeyvalType keyval);

    protected:

    		/**
    		 * Used to initialize null and predefined infos.
    		 * @see TrackBase::createPredefinedInfo.
    		 * (Implementation of hook)
    		 */
    		Keyval* createPredefinedInfo (int predef, MustKeyvalType handle);

    		/**
    		 * Returns a string for a predefined keyvalue.
    		 */
    		std::string getPredefinedName (MustMpiKeyvalPredefined predefined);

    }; /*class KeyvalTrack */
} /*namespace MUST*/

#endif /*KEYVALTRACK_H*/
