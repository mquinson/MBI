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
 * @file OpTrack.h
 *       @see MUST::OpTrack
 *
 *  @date 10.05.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "TrackBase.h"

#include "I_OpTrack.h"
#include "Op.h"

#include <map>

#ifndef OPTRACK_H
#define OPTRACK_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_OpTrack.
     */
    class OpTrack : public TrackBase<Op, I_Op, MustOpType, MustMpiOpPredefined, OpTrack, I_OpTrack>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		OpTrack (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~OpTrack (void);

    		/**
    		 * @see I_OpTrack::opCreate
    		 */
    		GTI_ANALYSIS_RETURN opCreate (
    				MustParallelId pId,
    				MustLocationId lId,
    				int commute,
    				MustOpType newOp);

    		/**
    		 * @see I_OpTrack::opFree
    		 */
    		GTI_ANALYSIS_RETURN opFree (
    				MustParallelId pId,
    				MustLocationId lId,
    				MustOpType op);

    		/**
    		 * @see I_OpTrack::getOp
    		 */
    		I_Op* getOp (
    				MustParallelId pId,
    				MustOpType op);

    		/**
    		 * @see I_OpTrack::getOp
    		 */
    		I_Op* getOp (
    				int rank,
    				MustOpType op);

    		/**
    		 * @see I_OpTrack::getPersistentOp
    		 */
    		I_OpPersistent* getPersistentOp (
    		        MustParallelId pId,
    		        MustOpType op);

    		/**
    		 * @see I_OpTrack::getPersistentOp
    		 */
    		I_OpPersistent* getPersistentOp (
    		        int rank,
    		        MustOpType op);

    protected:

    		/**
    		 * @see I_OpTrack::getPredefinedName
    		 */
    		std::string getPredefinedName (MustMpiOpPredefined predefined);

    		/**
    		 * Used to initialize predefined infos.
    		 * @see TrackBase::createPredefinedInfo.
    		 * (Implementation of hook)
    		 */
    		Op* createPredefinedInfo (int predefEnum, MustOpType handle);


    }; /*class OpTrack */
} /*namespace MUST*/

#endif /*GROUPTRACK_H*/
