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
 * @file I_CommReduction.h
 *       @see I_CommReduction.
 *
 *  @date 04.03.2011
 *  @author Mathias Korepkat, Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_Reduction.h"
#include "I_CommTrack.h"

#include <list>

#ifndef I_COMMREDUCTION_H
#define I_COMMREDUCTION_H

/**
 * Interface for a reduction running on the extra API
 * call used to propagate information about predefined
 * communicators and the processes that are reachable
 * by a place.
 * @see I_CommTrack::addPredefineds
 * @see I_CommPredefs::propagate
 *
 * Dependencies (order as listed):
 * X
 */
class I_CommReduction : public gti::I_Module, public gti::I_Reduction
{
public:

	/**
	 * The reduction function, arguments similar
	 * to I_CommTrack::addPredefineds except
	 * the extra reduction arguments.
	 *
	 * Beware, MPI_COMM_WORLD can have a different value on each process if
	 * an MPI split module is used to replace it with another communicator.
	 *
	 * @param reachableBegin start of interval of processes reachable by this place.
    	 * @param reachableEnd end of interval of processes reachable by this place.
    	 * @param worldSize size of MPI_COMM_WORLD
    	 * @param commNull value of MPI_COMM_NULL.
    	 * @param commSelf value of MPI_COMM_SELF.
    	 * @param commWorld value of MPI_COMM_WORLD (The real value of the constant, even if virtualization is used to replace MPI_COMM_WORLD with a differnt comm).
    	 * @param numWorlds number of MPI_COMM_WORLD values in list.
    	 * @param worlds values for MPI_COMM_WORLD on each task (The handle value of the possibly replaced MPI_COMM_WORLD, it may differ between ranks, e.g. MVAPICH).
    	 * @param thisChannel @see gti::I_Reduction.
    	 * @param outFinishedChannels @see gti::I_Reduction.
	 */
    virtual gti::GTI_ANALYSIS_RETURN reduce (
            MustParallelId pId,
    		int reachableBegin,
    		int reachableEnd,
    		int worldSize,
    		MustCommType commNull,
    		MustCommType commSelf,
    		MustCommType commWorld,
    		int numWorlds,
    		MustCommType *worlds,
    		gti::I_ChannelId *thisChannel,
    		std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

};/*class I_CommReduction*/

#endif /*I_COMMREDUCTION_H*/
