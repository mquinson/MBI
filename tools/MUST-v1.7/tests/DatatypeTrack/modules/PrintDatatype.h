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
 * @file PrintDatatype.h
 *       @see MUST::PrintDatatype.
 *
 *  @date 23.02.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_PrintDatatype.h"
#include "I_CreateMessage.h"

#ifndef PRINTDATATYPE_H
#define PRINTDATATYPE_H

using namespace gti;

namespace must
{
	/**
     * Implementation of I_PrintDatatype.
     */
    class PrintDatatype : public gti::ModuleBase<PrintDatatype, I_PrintDatatype>
    {
    public:
        /**
         * Constructor.
         * @param instanceName name of this module instance.
         */
    		PrintDatatype (const char* instanceName);

    		/**
    		 * Destructor.
    		 */
    		virtual ~PrintDatatype (void);

    		/**
    		 * @see I_PrintDatatype::print.
    		 */
    		GTI_ANALYSIS_RETURN print (
    		    		MustParallelId pId,
    		    		MustLocationId lId,
    		    		MustDatatypeType type);

    protected:
    		I_DatatypeTrack* myTypes;
    		I_CreateMessage* myLog;

    }; /*class PrintDatatype */
} /*namespace MUST*/

#endif /*PRINTDATATYPE_H*/
