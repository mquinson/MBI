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
 * @file I_PrintSendRecv.h
 *       @see I_PrintSendRecv.
 *
 *  @date 26.01.2011
 *  @author Tobias Hilbrich
 */

#include "I_Module.h"
#include "GtiEnums.h" //TODO Needs to be renamed to GTI enums

#ifndef I_PRINTSENDRECV_H
#define I_PRINTSENDRECV_H

/**
 * Interface for a module that prints information on
 * MPI_Sends and MPI_Recvs.
 */
class I_PrintSendRecv : public gti::I_Module
{
public:
	/**
	 * Prints information on a send or receive.
	 * @param count number of datatypes being sent or received.
	 * @param rank of calling process.
	 * @param sourceDest source for receives, destination rank for sends.
	 * @param isSend 1 if this is an MPI_Send, 0 otherwise (then it is assumed to be an MPI_Recv).
	 * @return see gti::GTI_ANALYSIS_RETURN.
	 */
    virtual gti::GTI_ANALYSIS_RETURN print (int count, int rank, int sourceDest, int isSend) = 0;

}; /*class I_Template*/

#endif /*I_PRINTSENDRECV_H*/
