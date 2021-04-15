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
 * @file GtiMacros.h
 *       Header file for common GTI macros.
 *
 * @author Tobias Hilbrich
 * @date 30.04.2009
 */

#ifndef GTI_MACROS_H
#define GTI_MACROS_H


#include <pnmpi/hooks.h>


namespace gti
{

/**
 * Creates a getInstance function for a module
 * that can have multiple instances.
 * TYPE is the class of the module.
 */
#define mGET_INSTANCE_FUNCTION(TYPE) \
	extern "C" int getInstance ## TYPE (void* pInstance, const void* instanceName) \
	{ \
		/*Create the instance*/ \
		*((gti::I_Module**)pInstance) = TYPE::getInstance((std::string)(const char*) instanceName); \
		return PNMPI_SUCCESS; \
	}

/**
 * Creates a freeInstance function for a module
 * that has multiple instances.
 * TYPE is the class of the module.
 */
#define mFREE_INSTANCE_FUNCTION(TYPE) \
	extern "C" int freeInstance ## TYPE  (void* pInstance) \
	{ \
		GTI_RETURN ret = TYPE::freeInstance((TYPE*)((gti::I_Module*) pInstance)); \
		assert (ret == GTI_SUCCESS); \
		return PNMPI_SUCCESS; \
	}

/**
 * Creates a freeInstance function for a module
 * for which not the last user, but the first
 * user calling freeInstance frees the module
 * instance.
 */
#define mFREE_INSTANCE_FUNCTION_FORCED(TYPE) \
	extern "C" int freeInstance ## TYPE  (void* pInstance) \
	{ \
		GTI_RETURN ret = TYPE::freeInstanceForced((TYPE*)((gti::I_Module*) pInstance)); \
		assert (ret == GTI_SUCCESS); \
		return PNMPI_SUCCESS; \
	}

/**
 * Creates a PNMPI_RegistrationPoint function for
 * a module.
 * NAME the name of the module.
 *
 * Warning: there is a almost identical version below,
 *          if something changes here it has to change there too!
 *
 */
#define mPNMPI_REGISTRATIONPOINT_FUNCTION(TYPE) \
	extern "C" int addDataHandler ## TYPE  (const void* instanceName, const void* key, const void* value) \
	{ \
		GTI_RETURN ret = TYPE::addData( \
				(std::string)(const char*) instanceName, \
				(std::string)(const char*) key, \
				(std::string)(const char*) value); \
		assert (ret == GTI_SUCCESS); \
		return PNMPI_SUCCESS; \
	} \
    \
	extern "C" void PNMPI_RegistrationPoint(void) \
	{ \
		static bool wasCalled = false; \
		if (wasCalled) \
		    return; \
	    wasCalled = true; \
     \
		int err = PNMPI_SUCCESS; \
		PNMPI_modHandle_t modHandle; \
		const char *modName; \
		PNMPI_Service_descriptor_t service; \
	 \
		err=PNMPI_Service_GetModuleSelf(&modHandle); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to get own module handle." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	 \
		/* get own module name */ \
		char temp[64]; \
		sprintf (temp, "moduleName"); \
		err=PNMPI_Service_GetArgument(modHandle, temp, &modName); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to get own module name." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	 \
		/* register this module and its getInstance/freeInstance function */ \
		err=PNMPI_Service_RegisterModule(modName); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to register as \"" << modName << "\"." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	 \
		sprintf(service.name,"getInstance"); \
		service.fct=(PNMPI_Service_Fct_t) getInstance ## TYPE ; \
		sprintf(service.sig,"pp"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to register getInstance function." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
     \
		sprintf(service.name,"freeInstance"); \
		service.fct=(PNMPI_Service_Fct_t) freeInstance ## TYPE ; \
		sprintf(service.sig,"p"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to register freeInstance function." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	 \
 		sprintf(service.name,"addDataHandler"); \
 		service.fct=(PNMPI_Service_Fct_t) addDataHandler ## TYPE ; \
 		sprintf(service.sig,"ppp"); \
 		err=PNMPI_Service_RegisterService(&service); \
 		if (err != PNMPI_SUCCESS) \
 		{ \
 			std::cerr << "Failed to register addDataHandler function." << std::endl; \
 			assert (err==PNMPI_SUCCESS); \
 		} \
 	 \
 		/* Read instance names from arguments and initialize them for the module class */ \
 		GTI_RETURN retM = TYPE::readModuleInstances(modHandle); \
		assert (retM == GTI_SUCCESS); \
	}

/**
 * Creates a PNMPI_RegistrationPoint function for
 * a module that is a wrapper. Thus offering the
 * getFunction service.
 * NAME the name of the module.
 */
#define mPNMPI_REGISTRATIONPOINT_FUNCTION_WRAPPER(TYPE) \
	extern "C" int addDataHandler ## TYPE  (const void* instanceName, const void* key, const void* value) \
	{ \
		GTI_RETURN ret = TYPE::addData( \
				(std::string)(const char*) instanceName, \
				(std::string)(const char*) key, \
				(std::string)(const char*) value); \
		assert (ret == GTI_SUCCESS); \
		return PNMPI_SUCCESS; \
	} \
    \
	extern "C" void PNMPI_RegistrationPoint(void) \
	{ \
		static bool wasCalled = false; \
		if (wasCalled) \
		    return; \
	    wasCalled = true; \
     \
		int err; \
		PNMPI_modHandle_t modHandle; \
		const char *modName; \
		PNMPI_Service_descriptor_t service; \
	 \
		err=PNMPI_Service_GetModuleSelf(&modHandle); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to get own module handle." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	 \
		/* get own module name */ \
		char temp[64]; \
		sprintf (temp, "moduleName"); \
		err=PNMPI_Service_GetArgument(modHandle, temp, &modName); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to get own module name." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	 \
		/* register this module and its getInstance/freeInstance function */ \
		err=PNMPI_Service_RegisterModule(modName); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to register as \"" << modName << "\"." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	 \
		sprintf(service.name,"getInstance"); \
		service.fct=(PNMPI_Service_Fct_t) getInstance ## TYPE ; \
		sprintf(service.sig,"pp"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to register getInstance function." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
     \
		sprintf(service.name,"freeInstance"); \
		service.fct=(PNMPI_Service_Fct_t) freeInstance ## TYPE ; \
		sprintf(service.sig,"p"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to register freeInstance function." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	 \
 		sprintf(service.name,"addDataHandler"); \
 		service.fct=(PNMPI_Service_Fct_t) addDataHandler ## TYPE ; \
 		sprintf(service.sig,"ppp"); \
 		err=PNMPI_Service_RegisterService(&service); \
 		if (err != PNMPI_SUCCESS) \
 		{ \
 			std::cerr << "Failed to register addDataHandler function." << std::endl; \
 			assert (err==PNMPI_SUCCESS); \
 		} \
 	 \
		sprintf(service.name,"getFunction"); \
		service.fct=(PNMPI_Service_Fct_t) getFunction ## TYPE ; \
		sprintf(service.sig,"pp"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
			std::cerr << "Failed to register getFunction function." << std::endl; \
			assert (err==PNMPI_SUCCESS); \
		} \
	\
		sprintf(service.name,"getAcrossFunction"); \
		service.fct=(PNMPI_Service_Fct_t) getAcrossFunction ## TYPE ; \
		sprintf(service.sig,"pp"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
		    std::cerr << "Failed to register getAcrossFunction function." << std::endl; \
		    assert (err==PNMPI_SUCCESS); \
		} \
    \
		sprintf(service.name,"getBroadcastFunction"); \
		service.fct=(PNMPI_Service_Fct_t) getBroadcastFunction ## TYPE ; \
		sprintf(service.sig,"pp"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
		    std::cerr << "Failed to register getBroadcastFunction function." << std::endl; \
		    assert (err==PNMPI_SUCCESS); \
		} \
	\
		sprintf(service.name,"getPlace"); \
		service.fct=(PNMPI_Service_Fct_t) getPlace ## TYPE ; \
		sprintf(service.sig,"p"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
		    std::cerr << "Failed to register getPlace function." << std::endl; \
		    assert (err==PNMPI_SUCCESS); \
		} \
	\
		sprintf(service.name,"getNextEventStrided"); \
		service.fct=(PNMPI_Service_Fct_t) getNextEventStridedFunction ## TYPE ; \
		sprintf(service.sig,"p"); \
		err=PNMPI_Service_RegisterService(&service); \
		if (err != PNMPI_SUCCESS) \
		{ \
		    std::cerr << "Failed to register getNextEventStrided function." << std::endl; \
		    assert (err==PNMPI_SUCCESS); \
		} \
	\
 		/* Read instance names from arguments and initialize them for the module class */ \
		GTI_RETURN retM = TYPE::readModuleInstances(modHandle); \
 		assert (retM == GTI_SUCCESS); \
	}

/**
 * Creates a function named "strategyRaisePanic" that notifies
 * this communication strategy of a panic (application crash).
 * TYPE -- the datatype of the module.
 */
#define mCOMM_STRATEGY_UP_RAISE_PANIC(TYPE) \
void strategyRaisePanic (void) \
{ \
    static bool hadPanic = false; \
    \
    /*Raise panic only once*/ \
    if (hadPanic) return; \
    hadPanic = true; \
\
    /*notify all existing communication strategies*/ \
    std::map <std::string, TYPE *> instances = TYPE::getActiveInstances (); \
    std::map <std::string, TYPE *>::iterator i; \
\
    for (i = instances.begin(); i != instances.end(); i++) \
    { \
        TYPE * strat = i->second; \
        if (strat) \
            strat->raisePanic(); \
    } \
}

} /*namespace gti*/

#endif /*GTI_MACROS_H*/
