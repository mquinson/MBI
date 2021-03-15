/*
 * GTI (Generic Tool Infrastructure)
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

/**
 * @file OMPTStartup.h
 * 
 *
 * @author Felix Münchhalfen
 * @date 09.07.2014
 *
 */
#include "ModuleBase.h"
#include "BaseApi.h"
#include "I_OMPTIntegration.h"

#ifndef OMPTSTARTUP_H
#define	OMPTSTARTUP_H

#ifdef GTI_OMPT_FOUND
#include <ompt.h>
#endif

namespace gti
{  
    /**
     * Class for the Integration of OMPT
     */
    class OMPTIntegration : public ModuleBase<OMPTIntegration, I_OMPTIntegration>
    {
    public:
        /**
         * Constructor.
         * Sets up the this tool place with all its modules.
         * @param instanceName name of this module instance
         */
        OMPTIntegration (const char* instanceName);
        /**
         * Destructor.
         */
        ~OMPTIntegration (void);
#ifdef GTI_OMPT_FOUND
        
        void OMPTInitialization( ompt_function_lookup_t fn_lookup );
        GTI_ANALYSIS_RETURN getLookUpFunction( ompt_function_lookup_t *func );
        GTI_ANALYSIS_RETURN setCallback( ompt_event_t event, ompt_callback_t callback);

        void ompt_init_callback( ompt_function_lookup_t ompt_fn_lookup,
                            const char *runtime_version,
                            unsigned int ompt_version
                          );
        
        static ompt_function_lookup_t myompt_fn_lookup;
        static ompt_set_callback_t myompt_fn_setcallback;
#endif        
    protected:
        

    }; /*class OMPTIntegration*/
} /*namespace gti*/

#endif	/* OMPTSTARTUP_H */
