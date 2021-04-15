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
 * @file GtiEnums.h
 *       Header file for global GTI enumerations.
 *
 * @author Tobias Hilbrich
 */


#ifndef GTI_ENUMS_H
#define GTI_ENUMS_H

namespace gti
{
    /**
     * Enumeration of gti return values.
     * @todo Add a mechanism to register an error message
     *       and offer a function to retrieve these.
     * @todo Do we need any further return values ?
     */
    typedef enum
    {
        GTI_SUCCESS = 0,
        GTI_ERROR,
        GTI_ERROR_NOT_INITIALIZED,
        GTI_ERROR_OUTSTANDING_LIMIT /*A Communication protocol uses this to say that it has too many open communications and could not accept a message (send/recv) */
    } GTI_RETURN;

    typedef enum
    {
        GTI_FLUSH,
        GTI_NO_FLUSH
    } GTI_FLUSH_TYPE;

    typedef enum
    {
        GTI_UNIFORM =0,
        GTI_BY_BLOCK
    } GTI_DISTRIBUTION;

    typedef enum
    {
        GTI_SYNC,
        GTI_NO_SYNC
    } GTI_SYNC_TYPE;

    /**
     * Enumeration of strategy types.
     */
    typedef enum
    {
        GTI_STRATEGY_UP = 0,
        GTI_STRATEGY_DOWN,
        GTI_STRATEGY_INTRA
    } GTI_STRATEGY_TYPE;

    /**
     * Enumeration of the return values used by analysis modules.
     * Use:
     * SUCCESS -- if analysis or analysis reduction is successfull and complete
     * FAILURE -- for critical errors that should lead to shutdown
     * WAITING -- for reduction analyses that started a reduction but are still waiting for further records to use in the reduction
     * IRREDUCIBLE -- for reductions that got all the inputs needed to perform a reduction but where an inconsistency in the data exists that makes the reduction impossible
     */
    typedef enum
    {
    		GTI_ANALYSIS_SUCCESS = 0,
    		GTI_ANALYSIS_FAILURE,
    		GTI_ANALYSIS_WAITING,
    		GTI_ANALYSIS_IRREDUCIBLE
    } GTI_ANALYSIS_RETURN;
}/*End of namespace gti*/

#endif /*GTI_ENUMS_H*/
