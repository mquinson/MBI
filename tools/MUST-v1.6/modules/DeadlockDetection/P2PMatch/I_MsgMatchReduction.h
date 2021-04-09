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
 * @file I_MsgMatchReduction.h
 *       @see I_MsgMatchReduction.
 *
 *  @date 17.03.2011
 *  @author Tobias Hilbrich, Mathias Korepkat
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_Reduction.h"

#ifndef I_MSGMATCHREDUCTION_H
#define I_MSGMATCHREDUCTION_H

/**
 * Interface for reducting sends and recvs for message matching.
 *
 * It checks whether this send/recv can be matched on this
 * place. If so, the LostMessage module is notified, otherwise
 * MUST_ANALYSIS_IRREDUCABLE is returned to forward
 * the message without any modification.
 *
 * @todo Limitations include:
 * - No support for MPI_ANY_SOURCE
 * - No support for MPI_Cancel
 *
 * Dependencies (order as listed):
 * - ParallelIdAnalysis
 * - LostMessage
 * - RequestTrack (for start[all])
 *
 */
class I_MsgMatchReduction : public gti::I_Module, public gti::I_Reduction
{
public:
	/**
	 * Notification of a started send.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param dest to send to.
	 * @param tag of the send.
	 * @param comm for the send.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN send (
			MustParallelId pId,
			MustLocationId lId,
			int dest,
			int tag,
			MustCommType comm,
            MustDatatypeType type,
            int count,
            int mode,
			gti::I_ChannelId *thisChannel,
			std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

	/**
	 * Notification of a started non-blocking send.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param dest to send to.
	 * @param tag of the send.
	 * @param comm for the send.
	 * @param request of the isend.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN isend (
			MustParallelId pId,
			MustLocationId lId,
			int dest,
			int tag,
			MustCommType comm,
            MustDatatypeType type,
            int count,
            int mode,
			MustRequestType request,
			gti::I_ChannelId *thisChannel,
			std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

	/**
	 * Motification of a started recv.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param source to receive from.
	 * @param tag of the receive.
	 * @param comm for the receive.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN recv (
			MustParallelId pId,
			MustLocationId lId,
			int source,
			int tag,
			MustCommType comm,
            MustDatatypeType type,
            int count,
			gti::I_ChannelId *thisChannel,
			std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

	/**
	 * Notification of a non-blocking receive.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param source to receive from.
	 * @param tag of the receive.
	 * @param comm for the receive.
	 * @param request for the non-blocking receive.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN irecv (
			MustParallelId pId,
			MustLocationId lId,
			int source,
			int tag,
			MustCommType comm,
            MustDatatypeType type,
            int count,
			MustRequestType request,
			gti::I_ChannelId *thisChannel,
			std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

	/**
	 * Notification of a started persistent send/recv operaiton.
	 *
	 * @param pId parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param request that was started.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
	virtual gti::GTI_ANALYSIS_RETURN startPersistent (
			MustParallelId pId,
			MustLocationId lId,
			MustRequestType request,
			gti::I_ChannelId *thisChannel,
			std::list<gti::I_ChannelId*> *outFinishedChannels) = 0;

};/*class I_MsgMatchReduction*/

#endif /*I_MsgMatchReduction_H*/
