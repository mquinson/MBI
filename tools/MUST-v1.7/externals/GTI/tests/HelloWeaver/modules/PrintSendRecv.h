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
 * @file PrintSendRecv.h
 *       @see gti::PrintSendRecv.
 *
 *  @date 26.01.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_PrintSendRecv.h"

#ifndef PRINTSENDRECV_H
#define PRINTSENDRECV_H

namespace gti
{
	/**
     * Implementation of I_PrintSendRecv.
     */
    class PrintSendRecv : public ModuleBase<PrintSendRecv, I_PrintSendRecv>
    {
    protected:

    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		PrintSendRecv (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~PrintSendRecv (void);

    		/**
    		 * @see I_PrintSendRecv::print.
    		 */
    		GTI_ANALYSIS_RETURN print (int count, int rank, int sourceDest, int isSend);

    }; /*class PrintSendRecv */
} /*namespace gti*/

#endif /*PRINTSENDRECV_H*/
