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
 * @file I_OpSplitter.h
 *       @see I_OpSplitter.
 *
 *  @date 05.04.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h"
#include "BaseIds.h"
#include "I_CommTrack.h"
#include "I_RequestTrack.h"

#ifndef I_TEMPLATE_H
#define I_TEMPLATE_H

/**
 * Is used in combination with MsgMatchReduction and LostMessage
 * to split MPI_Sendrecv calls into two distinct operations (one send op
 * and one recv op). A wrapp-everywhere call to which these two analyses
 * are mapped is used. Keep in mind that MsgMatchReduction and LostMessage
 * must thus not be mapped with their send/recv calls to MPI_Sendrecv or
 * MPI_Sendrecv_replace.
 *
 * May be extended to split MPI_Startall into multiple records too.
 *
 * Dependencies (order as listed):
 */
class I_OpSplitter : public gti::I_Module
{
public:

	/**
	 * Split sendrecv into multiple records.
	 *
	 * @param parallel Id of the call site.
	 * @param lId location Id of the call site.
	 * @param dest destination for send.
	 * @param sendtag tag of send.
	 * @param source for receive.
	 * @param recvtag tag for receive.
	 * @param comm used in the call.
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN splitSendRecv (
    		MustParallelId pId,
    		MustLocationId lId,
    		int dest,
    		int sendtag,
    		MustDatatypeType sendtype,
    		int sendcount,
    		int source,
    		int recvtag,
    		MustCommType recvtype,
    		int recvcount,
    		MustCommType comm) = 0;

    /**
     * Split MPI_Startall into multiple records.
     *
     * @param parallel Id of the call site.
     * @param lId location Id of the call site.
     * @param count number of requests being started.
     * @param requests to be started.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN splitStartall (
    		MustParallelId pId,
    		MustLocationId lId,
    		int count,
    		MustRequestType *requests) = 0;
};/*class I_OpSplitter*/

#endif /*I_OPSPLITTER_H*/
