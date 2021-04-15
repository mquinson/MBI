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
 * @file module_base.hxx
 *       @see gti::ModuleBase
 *
 * @author Tobias Hilbrich
 *
 * Complete overhaul done 20.10.2010.
 *
 */

#include <typeinfo>
#include <algorithm>
#include "I_Place.h"

#ifdef GTI_THREAD_SAFETY
//#include <thread>
#include <mutex>
#endif

#ifndef MODULE_BASE_HXX
#define MODULE_BASE_HXX

namespace gti
{

	//=============================
	// Static attribute construction: Accessor for ourModHandle
	//=============================
	template <class T, class Base, bool TLS>
	PNMPI_modHandle_t& ModuleBase<T, Base, TLS>::ourModHandle()
	{
		static PNMPI_modHandle_t modHandle = PNMPI_MODHANDLE_NULL;
		return modHandle;
	}

	//=============================
	// Static attribute construction: Accessor for ourModName
	//=============================
	template <class T, class Base, bool TLS>
  std::string& ModuleBase<T, Base, TLS>::ourModName()
	{
		static std::string modName;
		return modName;
	}

	//=============================
	// Constructor
	//=============================
	template <class T, class Base, bool TLS>
	ModuleBase<T, Base, TLS>::ModuleBase (const char* instanceName)
		: myRefCount (1),
  		  mySubModNames (),
		  myModData (),
		  myInstanceName (instanceName),
		  myLevelSizes (),
		  myDistributions (),
		  myBlocksizes (),
		  myOwnLevelId (-1)
	{
		//== Tasks
		//Read PNMPI Arguments
		// -> sub modules
		// -> instance data

		int index;
		const char* argument;
		int err;
		char temp[64];

		//== Find the index of this instance
		typename ModInstancesT::iterator instancePos;
		instancePos = (ModuleBase<T, Base, TLS>::ourInstances)().find (instanceName);
		assert (instancePos != (ModuleBase<T, Base, TLS>::ourInstances)().end());
		index = instancePos->second.second;

		//== sub modules
		sprintf (temp, "instance%dSubMods", index);
		err=PNMPI_Service_GetArgument(ModuleBase<T, Base, TLS>::ourModHandle(), temp, &argument);

		if (err == PNMPI_SUCCESS)
		{
			std::string subModArg (argument);
			size_t pos = 0, posLast;

			/*
			 * subModArg is of the form:
			 * x:y,z:w,...
			 * each pair a:b is a module and instance name pair
			 *
			 * This loop is used to extract each pair.
			 */
			do
			{
				std::string pairDesc, modName, instanceName;
				size_t columnPos;

				//find pair
				posLast = pos;
				pos = subModArg.find_first_of (',', pos);

				//Use remaining string if no ',' was found
				if (pos == std::string::npos)
					pos = subModArg.length();//emulates a ',' after the actual string

				//extract substring of form:
				//MOD_NAME:INSTANCE_NAME
				pairDesc.assign (subModArg,posLast,pos-posLast);

				//find the column character and split accordingly
				columnPos = pairDesc.find_first_of (':');

				if (columnPos == std::string::npos)
				{
					std::cerr
						<< "Error: a sub module string is malformed moduleName,instanceName pair was \""
						<< pairDesc
						<< "\" it needs to be of form MOD_NAME:INSTANCE_NAME, complete sub module argument was \""
						<< subModArg
						<< "\"."
						<< std::endl;
					assert (0);
				}

				modName.assign (pairDesc, 0, columnPos);
				instanceName.assign (pairDesc, columnPos+1, pairDesc.length() - columnPos - 1);
				mySubModNames.push_back (std::make_pair (modName, instanceName));

				//next
				pos++;

			} while (pos != std::string::npos && pos < subModArg.length());
		}/*found sub module argument*/

		//== Module data
		sprintf (temp, "instance%dData", index);
		err=PNMPI_Service_GetArgument(ModuleBase<T, Base, TLS>::ourModHandle(), temp, &argument);

		if (err == PNMPI_SUCCESS)
		{
			std::string dataArg (argument);
			size_t pos = 0, posLast;

			/*
			 * dataArg is of the form:
			 * x=y,z=w,...
			 * each pair a=b is a key value pair
			 *
			 * This loop is used to extract each pair.
			 */
			do
			{
				std::string pairDesc, key, value;
				size_t columnPos;

				//find pair
				posLast = pos;
				pos = dataArg.find_first_of (',', pos);

				//Use remaining string if no ',' was found
				if (pos == std::string::npos)
					pos = dataArg.length();//emulates a ',' after the actual string

				//extract substring of form:
				//MOD_NAME:INSTANCE_NAME
				pairDesc.assign (dataArg,posLast,pos-posLast);

				//find the column character and split accordingly
				columnPos = pairDesc.find_first_of ('=');

				if (columnPos == std::string::npos)
				{
					std::cerr
					<< "Error: a sub module string is malformed moduleName,instanceName pair was \""
					<< pairDesc
					<< "\" it needs to be of form MOD_NAME:INSTANCE_NAME, complete sub module argument was \""
					<< dataArg
					<< "\"."
					<< std::endl;
					assert (0);
				}

				key.assign (pairDesc, 0, columnPos);
				value.assign (pairDesc, columnPos+1, pairDesc.length() - columnPos - 1);
				myModData.insert (std::make_pair(key, value));

				//next
				pos++;

			} while (pos != std::string::npos && pos < dataArg.length());
		}

		//== Add data given from ancestors
		DataMapT extraData =  (ModuleBase<T, Base, TLS>::ourDataFromAncestors)()[instanceName];
		DataMapT::iterator dataIter;

		for (dataIter = extraData.begin(); dataIter != extraData.end(); dataIter++)
		{
			if (myModData.find (dataIter->first) == myModData.end())
				myModData.insert (*dataIter);
			else
				myModData[dataIter->first] = dataIter->second;
		}

		//== Add ancestor data to my sub modules
		for (dataIter = extraData.begin(); dataIter != extraData.end(); dataIter++)
		{
			addDataToSubmodules (dataIter->first, dataIter->second);
		}

	}

	//=============================
	// getInstance
	//=============================
	template <class T, class Base, bool TLS>
	T* ModuleBase<T, Base, TLS>::getInstance (std::string instanceName)
	{
		typename ModInstancesT::iterator pos;

		/*Dummy call -> will cause init of PnMPI if that hasn't happened yet ...*/
		PNMPI_modHandle_t modHandle;
		PNMPI_Service_GetModuleSelf(&modHandle);

		//If name is an empty string, we return the first instance
		if (instanceName == "")
		{
			for (pos = (ModuleBase<T, Base, TLS>::ourInstances)().begin(); pos != (ModuleBase<T, Base, TLS>::ourInstances)().end(); pos++)
			{
				if (pos->second.second == 0)
				{
					instanceName = pos->first;
					break;
				}
			}
		}

		//find instance name
		pos = (ModuleBase<T, Base, TLS>::ourInstances)().find (instanceName);

		//instance name known ?
		if (pos == (ModuleBase<T, Base, TLS>::ourInstances)().end())
		{
			if (instanceName != ""){
				std::cerr << "Unknown instance name \"" << instanceName << "\" of class " << typeid(T).name() << getpid () << std:: endl;

				std::cerr << "Known instances: " << std::endl;
				for (pos = (ModuleBase<T, Base, TLS>::ourInstances)().begin(); pos != (ModuleBase<T, Base, TLS>::ourInstances)().end(); pos++)
				{
					std::cerr << " * " << pos->first << std::endl;
				}
			}
			return NULL;
		}

		//existing instance ?
		if (pos->second.first != NULL)
			pos->second.first->myRefCount = pos->second.first->myRefCount + 1;
		else
			pos->second.first = new T (instanceName.c_str());

		return pos->second.first;
	}

	//=============================
	// freeInstance
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::freeInstance (T* instance)
	{
		assert (instance);

		instance->myRefCount = instance->myRefCount - 1;

		if (instance->myRefCount == 0)
		{
		  typename ModInstancesT::iterator iter = (ModuleBase<T, Base, TLS>::ourInstances)().find(instance->myInstanceName);
		  if(iter != (ModuleBase<T, Base, TLS>::ourInstances)().end())
		  {
				  //Instance was consumed, completely remove it from registration
				  (ModuleBase<T, Base, TLS>::ourInstances)().erase (iter);

			}
			delete instance;
		  
		}
	  return GTI_SUCCESS;

	}

	//=============================
	// freeInstanceForced
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::freeInstanceForced (T* instance)
	{
                assert (instance);
		instance->myRefCount = 1;
		return freeInstance (instance);
	}


	template <class T, class Base, bool TLS>
	void ModuleBase<T, Base, TLS>::initModuleOnce (PNMPI_modHandle_t modHandle)
  {
		int err;
		char temp[64];
		const char *modName;
		// store our static module handle
		ModuleBase<T, Base, TLS>::ourModHandle() = modHandle;

		// get own module name
		sprintf (temp, "moduleName");
		err=PNMPI_Service_GetArgument(modHandle, temp, &modName);
		assert (err == PNMPI_SUCCESS);
		ModuleBase<T, Base, TLS>::ourModName() = modName;
  }


	//=============================
	// readModuleInstances
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::readModuleInstances (PNMPI_modHandle_t modHandle)
	{
#ifdef USE_THREAD_LOCAL
    static thread_local bool initialized=false;
    if (initialized)
      return GTI_SUCCESS;
#else
          static sf::contfree_safe_ptr<std::vector<bool>> inited{};
          int tid = getGtiTid();
          {
            auto x_safe_inited = xlock_safe_ptr(inited);
            if (x_safe_inited->size() < tid + 1)
              x_safe_inited->resize(tid + 1, false);
            if (x_safe_inited->at(tid))
              return GTI_SUCCESS;
          }
#endif
		int err;
		const char *argument;
		int numInstances;
		char temp[64];

#ifdef GTI_THREAD_SAFETY
    static std::once_flag key_once;
    std::call_once(key_once, ModuleBase<T, Base, TLS>::initModuleOnce, modHandle);
#else
    initModuleOnce(modHandle);
#endif

		const char *modName = ModuleBase<T, Base, TLS>::ourModName().c_str();

		// get number of instances
		sprintf (temp, "numInstances");
		err=PNMPI_Service_GetArgument(modHandle, temp, &argument);

		if (err != PNMPI_SUCCESS)
		{
			//No instances given
			std::cerr << "Warning: module named \"" << modName << "\" has no numInstances argument, thus it can't be instantiated." << std::endl;
			return GTI_SUCCESS;
		}

		numInstances = atoi (argument);
                //std::cout << "insert " << argument << " instances to ourInstances of " << typeid(T).name() << std::endl;

    if (numInstances>0)
#ifdef USE_THREAD_LOCAL
        initialized=true;
#else
      xlock_safe_ptr(inited)->at(tid) = true;
#endif
    
		// read all the instance names
		for (int i = 0; i < numInstances; i++)
		{
			sprintf (temp, "instance%d", i);

			// Read name of instance
			err=PNMPI_Service_GetArgument(modHandle, temp, &argument);

			if (err != PNMPI_SUCCESS)
			{
				std::cerr << "Error: module named \"" << modName << "\" specifies " << numInstances << " instances but no name for instance of index " << i << " is given." << std::endl;
				return GTI_ERROR;
			}

			// Add to map of known instances
                        //std::cout << "insert " << argument << " to ourInstances of " << typeid(T).name() << std::endl;
			(ModuleBase<T, Base, TLS>::ourInstances)().insert (std::make_pair(argument, std::make_pair((T*)NULL, i)));

			// Add an item to static attribute for extra module instance data
			std::map<std::string, std::string> empty;
			(ModuleBase<T, Base, TLS>::ourDataFromAncestors)().insert (std::make_pair(argument, empty));
		}

		return GTI_SUCCESS;
	}

	//=============================
	// addData
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::addData (
			std::string instanceName,
			std::string key,
			std::string value)
	{
		typename std::map<std::string, DataMapT >::iterator pos;

		//Find map entry for instance
		pos = (ModuleBase<T, Base, TLS>::ourDataFromAncestors)().find (instanceName);

		if (pos == (ModuleBase<T, Base, TLS>::ourDataFromAncestors)().end())
		{
			std::cerr << "In ModuleBase::addData: invalid instance Name given. (name=" << instanceName << ")" << std::endl;
			return GTI_ERROR;
		}

		//Find entry for key (if present) and set value
		DataMapT::iterator i;

		i = pos->second.find (key);

		if (i == pos->second.end())
			pos->second.insert (std::make_pair (key, value));
		else
			(pos->second)[key] = value;

		return GTI_SUCCESS;
	}

	//=============================
	// getActiveInstances
	//=============================
	template <class T, class Base, bool TLS>
	std::map <std::string, T*> ModuleBase<T, Base, TLS>::getActiveInstances (void)
	{
		std::map <std::string, T*> ret;

		typename ModInstancesT::iterator i;

		for (i = (ModuleBase<T, Base, TLS>::ourInstances)().begin(); i != (ModuleBase<T, Base, TLS>::ourInstances)().end(); i++)
		{
			if (i->second.first) //only insert if active !
				ret.insert (std::make_pair (i->first, i->second.first));
		}

		return ret;
	}

	//=============================
	// getWrapperHandle
	//=============================
	template <class T, class Base, bool TLS>
	PNMPI_modHandle_t ModuleBase<T, Base, TLS>::getWrapperHandle()
	{
#ifdef USE_THREAD_LOCAL
	static thread_local PNMPI_modHandle_t handle=-1;
#else
          static sf::contfree_safe_ptr<std::vector<PNMPI_modHandle_t>>
              handle_vector{};
          int tid = getGtiTid();
          auto x_safe_handle_vector = xlock_safe_ptr(handle_vector);
          if (x_safe_handle_vector->size() < tid + 1)
            x_safe_handle_vector->resize(tid + 1, -1);
          PNMPI_modHandle_t &handle = x_safe_handle_vector->at(tid);
#endif
	    if (handle!=-1)
		return handle;
	    int err;
	    int instanceIndex;
	    const char *wrappModName;


	    //Get module handle
#ifdef PNMPI_FIXED
	    err = PNMPI_Service_GetModuleByName((ModuleBase<T, Base, TLS>::ourModName)().c_str(), &handle);
#else
	    char temp[64];
	    sprintf (temp, "%s", (ModuleBase<T, Base, TLS>::ourModName)().c_str());
	    err = PNMPI_Service_GetModuleByName(temp, &handle);
#endif
	    assert (err == PNMPI_SUCCESS);
	    if (err != PNMPI_SUCCESS) return handle;

	    //Get own module index
	    typename ModInstancesT::iterator pos = (ModuleBase<T, Base, TLS>::ourInstances)().find (myInstanceName);
	    assert (pos != (ModuleBase<T, Base, TLS>::ourInstances)().end()); //should never happen
	    instanceIndex = pos->second.second;

	    //Get wrapper argument
	    char wrapperArg[128];
	    sprintf (wrapperArg, "instance%dWrapper", instanceIndex);
	    err=PNMPI_Service_GetArgument(handle, wrapperArg, &wrappModName);
	    assert (err == PNMPI_SUCCESS);
	    if (err != PNMPI_SUCCESS) return handle;

	    //Get wrapper module
	    err = PNMPI_Service_GetModuleByName(wrappModName, &handle);
	    return handle;
	}

	//=============================
	// getWrapperService
	//=============================
	template <class T, class Base, bool TLS>
	PNMPI_Service_descriptor_t ModuleBase<T, Base, TLS>::getWrapperService(const char * serviceName, const char * serviceSig)
	{
	    int err;
	    PNMPI_modHandle_t handle = getWrapperHandle();
	    assert(handle != -1);
	    PNMPI_Service_descriptor_t service;
	    //Get service
#ifdef PNMPI_FIXED
	    err = PNMPI_Service_GetServiceByName(handle, serviceName, serviceSig, &service);
#else
	    char temp[64];
	    char temp2[3];
	    sprintf (temp2, "%s", serviceSig);
	    sprintf (temp, "%s", serviceName);
	    err = PNMPI_Service_GetServiceByName(handle, temp, temp2, &service);
#endif
	    assert (err == PNMPI_SUCCESS);
//	    if (err != PNMPI_SUCCESS) return GTI_ERROR;
	    return service;
	}

	//=============================
	// createSubModuleInstances
	//=============================
	template <class T, class Base, bool TLS>
	std::vector<I_Module*> ModuleBase<T, Base, TLS>::createSubModuleInstances (void)
	{
		PNMPI_modHandle_t handle;
		PNMPI_Service_descriptor_t service;
		int err;
		std::vector<I_Module*> subModInstances;

		//Create the given sub modules and initialize them
		for (SubModNamesT::iterator i = mySubModNames.begin(); i != mySubModNames.end(); i++)
		{
#ifdef PNMPI_FIXED
			err = PNMPI_Service_GetModuleByName(i->first.c_str(), &handle);

#else
			char string[512];
			sprintf (string, "%s",i->first.c_str());
			err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
			if (err != PNMPI_SUCCESS)
			{
				std::cerr
				<< "Failed to get a handle for the P^nMPI module \""
				<< i->first
				<< "\""
				<< std::endl
				<< "(Failed in module:instance \""
				<< getName()
				<< ":"
				<< myInstanceName
				<< "\")"
				<< std::endl;
				assert (0);
			}

#ifdef PNMPI_FIXED
			err = PNMPI_Service_GetServiceByName(handle, "getInstance", "pp", &service);
#else
			char string2[3];
			sprintf (string2, "pp");
			sprintf (string, "getInstance");
			err = PNMPI_Service_GetServiceByName(handle, string, string2, &service);
#endif
			assert (err == PNMPI_SUCCESS);

			I_Module* instance;
			((getInstance_t) service.fct) (&instance, i->second.c_str());
			if((!TLS && instance->usesTLS())){
				printf("%s uses TLS, while this module (%s) does not\n", i->first.c_str(), __PRETTY_FUNCTION__);
				//assert(0);
			}
			subModInstances.push_back(instance);
		}

		return subModInstances;
	}

#if 0
	//=============================
	// getInstanceByName
	//=============================
	template <class T, class Base, bool TLS>
	I_Module* ModuleBase<T, Base, TLS>::getInstanceByName (std::string name)
	{
		PNMPI_modHandle_t handle;
		PNMPI_Service_descriptor_t service;
		int err;
#ifdef PNMPI_FIXED
			err = PNMPI_Service_GetModuleByName(name, &handle);

#else
			char string[512];
			sprintf (string, "%s",name);
			err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
			if (err != PNMPI_SUCCESS)
			{
				std::cerr
				<< "Failed to get a handle for the P^nMPI module \""
				<< name
				<< "\""
				<< std::endl
				<< "(Failed in module:instance \""
				<< getName()
				<< ":"
				<< myInstanceName
				<< "\"::getInstanceByName)"
				<< std::endl;
				assert (0);
			}

#ifdef PNMPI_FIXED
			err = PNMPI_Service_GetServiceByName(handle, "getInstance", "pp", &service);
#else
			char string2[3];
			sprintf (string2, "pp");
			sprintf (string, "getInstance");
			err = PNMPI_Service_GetServiceByName(handle, string, string2, &service);
#endif
			assert (err == PNMPI_SUCCESS);

			I_Module* instance;
			((getInstance_t) service.fct) (&instance, i->second.c_str());

		return instance;
	}
#endif

	//=============================
	// destroySubModuleInstance
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::destroySubModuleInstance (I_Module* instance)
	{
		PNMPI_modHandle_t handle;
		PNMPI_Service_descriptor_t service;
		int err;

/*//DEBUG
std::cerr << "Calling destroySubModuleInstance: " << (uint64_t) instance << std::endl;
*/
#ifdef PNMPI_FIXED
		err = PNMPI_Service_GetModuleByName(instance->getName().c_str(), &handle);

#else
		char string[512];
		sprintf (string, "%s",((I_Module*)instance)->getName().c_str());
		err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
		assert (err == PNMPI_SUCCESS);

#ifdef PNMPI_FIXED
		err = PNMPI_Service_GetServiceByName(handle, "freeInstance", "p", &service);
#else
		char string2[2];
		sprintf (string2, "p");
		sprintf (string, "freeInstance");
		err = PNMPI_Service_GetServiceByName(handle, string, string2, &service);
#endif

		assert (err == PNMPI_SUCCESS);
		((freeInstance_t) service.fct) (instance);

		return GTI_SUCCESS;
	}

	//=============================
	// getData
	//=============================
	template <class T, class Base, bool TLS>
	typename ModuleBase<T, Base, TLS>::DataMapT ModuleBase<T, Base, TLS>::getData (void)
	{
		return myModData;
	}

	//=============================
	// addDataToSubmodules
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::addDataToSubmodules (std::string key, std::string value)
	{
		SubModNamesT::iterator i;

		for (i = mySubModNames.begin(); i != mySubModNames.end(); i++)
		{
			PNMPI_modHandle_t handle;
			PNMPI_Service_descriptor_t service;
			int err;

			std::string
				moduleName = i->first,
				instanceName = i->second;

#ifdef PNMPI_FIXED
			err = PNMPI_Service_GetModuleByName(moduleName.c_str(), &handle);

#else
			char string[512];
			sprintf (string, "%s",moduleName.c_str());
			err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
			if (err != PNMPI_SUCCESS)
			{
				std::cerr
				<< "Failed to get a handle for the P^nMPI module \""
				<< moduleName
				<< "\""
				<< std::endl
				<< "(Failed in module:instance \""
				<< getName()
				<< ":"
				<< myInstanceName
				<< "\")"
				<< std::endl;
				assert (0);
			}

#ifdef PNMPI_FIXED
			err = PNMPI_Service_GetServiceByName(handle, "addDataHandler", "ppp", &service);
#else
			char string2[4];
			sprintf (string2, "ppp");
			sprintf (string, "addDataHandler");
			err = PNMPI_Service_GetServiceByName(handle, string, string2, &service);
#endif
			assert (err == PNMPI_SUCCESS);

			((addDataHandler_t) service.fct) (instanceName.c_str(), key.c_str(), value.c_str());
		}

		return GTI_SUCCESS;
	}

	//=============================
	// getWrapperFunction
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getWrapperFunction (
			std::string functionName,
			GTI_Fct_t* pOutFunction)
	{
	    int err;
	    if (pOutFunction) *pOutFunction = NULL;
#ifdef USE_THREAD_LOCAL
	    static thread_local PNMPI_Service_descriptor_t service;
	    static thread_local bool isInitialized=false;
            bool initialized = isInitialized;
            if (!initialized)
              isInitialized = true;
#else
            static sf::contfree_safe_ptr<
                std::vector<PNMPI_Service_descriptor_t>>
                service_vector{};
            static sf::contfree_safe_ptr<std::vector<bool>>
                isInitialized_vector{};
            bool initialized;
            int tid = getGtiTid();
            {
              auto x_safe_inited = xlock_safe_ptr(isInitialized_vector);
              if (x_safe_inited->size() < tid + 1)
                x_safe_inited->resize(tid + 1, false);
              initialized = x_safe_inited->at(tid);
              if (!initialized)
                x_safe_inited->at(tid) = true;
            }
            auto x_safe_service_vector = xlock_safe_ptr(service_vector);
            if (x_safe_service_vector->size() < tid + 1)
              x_safe_service_vector->resize(tid + 1);
            PNMPI_Service_descriptor_t &service =
                x_safe_service_vector->at(tid);
#endif
            if (!initialized) {
              service = getWrapperService("getFunction", "pp");
            }

                //Call getFunction to get symbol
		if (((getFunction_t) service.fct) (functionName.c_str(), pOutFunction) != PNMPI_SUCCESS)
			return GTI_ERROR;

		return GTI_SUCCESS;
	}

	//=============================
	// getWrapAcrossFunction
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getWrapAcrossFunction (
	        std::string functionName,
	        GTI_Fct_t* pOutFunction)
	{
	    int err;
	    if (pOutFunction) *pOutFunction = NULL;
#ifdef USE_THREAD_LOCAL
	    static thread_local PNMPI_Service_descriptor_t service;
	    static thread_local bool isInitialized=false;
            bool initialized = isInitialized;
            if (!initialized)
              isInitialized = true;
#else
            static sf::contfree_safe_ptr<
                std::vector<PNMPI_Service_descriptor_t>>
                service_vector{};
            static sf::contfree_safe_ptr<std::vector<bool>>
                isInitialized_vector{};
            bool initialized;
            int tid = getGtiTid();
            {
              auto x_safe_inited = xlock_safe_ptr(isInitialized_vector);
              if (x_safe_inited->size() < tid + 1)
                x_safe_inited->resize(tid + 1, false);
              initialized = x_safe_inited->at(tid);
              if (!initialized)
                x_safe_inited->at(tid) = true;
            }
            auto x_safe_service_vector = xlock_safe_ptr(service_vector);
            if (x_safe_service_vector->size() < tid + 1)
              x_safe_service_vector->resize(tid + 1);
            PNMPI_Service_descriptor_t &service =
                x_safe_service_vector->at(tid);

#endif
            if (!initialized) {
              service = getWrapperService("getAcrossFunction", "pp");
            }

            //Call getFunction to get symbol
	    if (((getFunction_t) service.fct) (functionName.c_str(), pOutFunction) != PNMPI_SUCCESS)
	        return GTI_ERROR;

	    return GTI_SUCCESS;
	}

	//=============================
	// getBroadcastFunction
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getBroadcastFunction (
	        std::string functionName,
	        GTI_Fct_t* pOutFunction)
    {
	    int err;
	    if (pOutFunction) *pOutFunction = NULL;
#ifdef USE_THREAD_LOCAL
	    static thread_local PNMPI_Service_descriptor_t service;
	    static thread_local bool isInitialized=false;
            bool initialized = isInitialized;
            if (!initialized)
              isInitialized = true;
#else
            static sf::contfree_safe_ptr<
                std::vector<PNMPI_Service_descriptor_t>>
                service_vector{};
            static sf::contfree_safe_ptr<std::vector<bool>>
                isInitialized_vector{};
            bool initialized;
            int tid = getGtiTid();
            {
              auto x_safe_inited = xlock_safe_ptr(isInitialized_vector);
              if (x_safe_inited->size() < tid + 1)
                x_safe_inited->resize(tid + 1, false);
              initialized = x_safe_inited->at(tid);
              if (!initialized)
                x_safe_inited->at(tid) = true;
            }
            auto x_safe_service_vector = xlock_safe_ptr(service_vector);
            if (x_safe_service_vector->size() < tid + 1)
              x_safe_service_vector->resize(tid + 1);
            PNMPI_Service_descriptor_t &service =
                x_safe_service_vector->at(tid);
#endif
            if (!initialized) {
              service = getWrapperService("getBroadcastFunction", "pp");
            }

            //Call getFunction to get symbol
	    if (((getFunction_t) service.fct) (functionName.c_str(), pOutFunction) != PNMPI_SUCCESS)
	        return GTI_ERROR;

	    return GTI_SUCCESS;
	}

	//=============================
	// getSetNextEventStridedFunction
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getSetNextEventStridedFunction (GTI_Fct_t* pOutFunction)
	{
	    int err;
	    if (pOutFunction) *pOutFunction = NULL;
#ifdef USE_THREAD_LOCAL
	    static thread_local PNMPI_Service_descriptor_t service;
	    static thread_local bool isInitialized=false;
            bool initialized = isInitialized;
            if (!initialized)
              isInitialized = true;
#else
            static sf::contfree_safe_ptr<
                std::vector<PNMPI_Service_descriptor_t>>
                service_vector{};
            static sf::contfree_safe_ptr<std::vector<bool>>
                isInitialized_vector{};
            bool initialized;
            int tid = getGtiTid();
            {
              auto x_safe_inited = xlock_safe_ptr(isInitialized_vector);
              if (x_safe_inited->size() < tid + 1)
                x_safe_inited->resize(tid + 1, false);
              initialized = x_safe_inited->at(tid);
              if (!initialized)
                x_safe_inited->at(tid) = true;
            }
            auto x_safe_service_vector = xlock_safe_ptr(service_vector);
            if (x_safe_service_vector->size() < tid + 1)
              x_safe_service_vector->resize(tid + 1);
            PNMPI_Service_descriptor_t &service =
                x_safe_service_vector->at(tid);
#endif
            if (!initialized) {
              service = getWrapperService("getNextEventStrided", "p");
            }

            //Call getFunction to get symbol
	    if (((getFunctionNoName_t) service.fct) (pOutFunction) != PNMPI_SUCCESS)
	        return GTI_ERROR;

	    return GTI_SUCCESS;

	}

	//=============================
	// getLevelIdForApplicationRank
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getLevelIdForApplicationRank (
	        int rank,
	        int *outLevelId)
	{
	    //Do we need to read level sizes and our own index?
	    if (myOwnLevelId < 0)
	    {
	        //Get our own level index
	        if (myModData.find("gti_own_level") == myModData.end())
	        {
	            std::cerr << "Error: no \"gti_own_level\" data field was specified for this module! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
	            return GTI_ERROR;
	        }

	        myOwnLevelId = atoi (myModData.find("gti_own_level")->second.c_str());

	        //Read the sizes and distributions of all ancestor levels
	        myLevelSizes.resize(myOwnLevelId+1);
	        myBlocksizes.resize(myOwnLevelId);
	        myDistributions.resize(myOwnLevelId);
		std::vector<bool> isMpiLevel;
		isMpiLevel.resize(myOwnLevelId+1);

	        for (int i = 0; i <= myOwnLevelId; i++)
	        {
	            std::stringstream mpiName;
	            mpiName << "gti_level_" << i << "_mpi";
	            if (myModData.find(mpiName.str()) == myModData.end())
			isMpiLevel[i]=false;
                    else
			isMpiLevel[i]=true;


	            //Read level size
	            std::stringstream keyName;
	            keyName << "gti_level_" << i << "_size";
	            if (myModData.find(keyName.str()) == myModData.end())
	            {
	                std::cerr << "Error: no \"" << keyName.str() << "\" data field was specified for this module, even though the \"gti_own_level\" value indicates its presence! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
	                return GTI_ERROR;
	            }

	            myLevelSizes[i] = atoi (myModData.find(keyName.str())->second.c_str());
                   
                    
	            //Read distribution
	            if (i == myOwnLevelId) break;

	            std::stringstream distribName;
	            GTI_DISTRIBUTION distrib = GTI_UNIFORM;
	            distribName << "gti_level_" << i << "_" << i+1 << "_distribution";
	            if (myModData.find(distribName.str()) != myModData.end())
	            {
	                if (myModData.find(distribName.str())->second == "by-block")
	                {
	                    distrib = GTI_BY_BLOCK;

	                    //Also read the blocksize
	                    std::stringstream bsizeName;
	                    bsizeName << "gti_level_" << i << "_" << i+1 << "_blocksize";
	                    if (myModData.find(bsizeName.str()) == myModData.end())
	                    {
	                        std::cerr << "A block distribution was specified in " << distribName.str() << " but no blocksize was given! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
	                        return GTI_ERROR;
	                    }
	                    myBlocksizes[i] =  atoi (myModData.find(bsizeName.str())->second.c_str());
	                }
	                else if (myModData.find(distribName.str())->second == "uniform")
	                {
	                    distrib = GTI_UNIFORM;
	                }
	                else
	                {
	                    std::cerr << "Error: \"" << myModData.find(distribName.str())->second << "\" is an unknown distribution value set for gti_level_" << i << "_" << i+1 << "_distribution. (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
	                    return GTI_ERROR;
	                }
	            }

	            myDistributions[i] = distrib;
	        }
		for (int i = myOwnLevelId-1; i >= 0; i--)
		{
		    int reduce=1;
		    if(!isMpiLevel[i])
		    {
			if(myLevelSizes[i] % (myLevelSizes[i+1]*reduce) != 0){
	                    std::cerr << "Error: cannot handle non-MPI level, that doesn't evenly distribute on next level" << std::endl;
			    assert(false);
			}
			reduce = myLevelSizes[i]/myLevelSizes[i+1];
		    }
		    if(myLevelSizes[i] % reduce != 0){
	                std::cerr << "Error: cannot handle non-MPI level, that doesn't evenly distribute on next level" << std::endl;
			assert(false);
		    }
		    myLevelSizes[i] /= reduce;
		}
	    }//Parsing data necessary?

	    //Now calculate the place id for the given rank
	    int currentIndex = rank;

	    if (currentIndex >= myLevelSizes[0])
	    {
	        //Out of range
	        return GTI_ERROR;
	    }

	    //We propagate the index level wise
	    for (int i = 1; i < myLevelSizes.size(); i++)
	    {
	        if (myDistributions[i-1] == GTI_UNIFORM)
	        {
                int sB = myLevelSizes[i-1];
                int sT= myLevelSizes[i];
                // if we have a ghostlevel for threads here
                // take the next one for the top level!
                if (sT == 0)
                {
                    i++;
                    if( i >= myLevelSizes.size())
                        break;
                    sT = myLevelSizes[i];
                }

                int numBperT = sB / sT;

                // TODO: Necessary?
                if (numBperT == 0)
                {
                    std::cerr << "ERROR: rank distribution calculation does not support cases where a top layer is larger than its bottom layer! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
                    return GTI_ERROR;
                }

                int numRemaining = sB - numBperT * sT; //number of ranks that can't be distributed evenly across the top layer

                if (currentIndex < (numBperT+1) * numRemaining)
                {
                    //We are within the range where remaining tasks are distributed
                    currentIndex = currentIndex / (numBperT+1);
                }
                else
                {
                    //We are after the range where remaining tasks are all used up
                    currentIndex  = currentIndex - numRemaining*(numBperT+1);
                    currentIndex = currentIndex / numBperT;
                    currentIndex += numRemaining;
                }
	        }
	        else if (myDistributions[i-1] == GTI_BY_BLOCK)
	        {
                    if( myBlocksizes[i-1] == 0)
                        continue;
	            currentIndex = currentIndex/myBlocksizes[i-1];
	        }
	    }

	    if (outLevelId)
	        *outLevelId = currentIndex;

	    return GTI_SUCCESS;
	}

	//=============================
	// getReachableRanks
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getReachableRanks (int *outBegin, int *outEnd, int rank)
	{
	    int ownId;

	    //Which of the TBON nodes on this layer are we?
	    if (getLevelIdForApplicationRank (rank, &ownId) != GTI_SUCCESS)
	        return GTI_ERROR;

        //Now calculate the minimum and maxium rank we can get events from
        //We propagate the index level wise
	    int currentTopIndex = ownId;
	    int currentBottomIndex = ownId;
        for (int i = myOwnLevelId - 1; i >= 0; i--)
        {
            if (myDistributions[i] == GTI_UNIFORM)
            {
                int sT= myLevelSizes[i+1];
                int sB = myLevelSizes[i];
                if (sT == 0)
                {
                    sT = 1;
                }
                if(sB == 0)
                {
                    sB = sT;
                }
                
                int numBperT = sB / sT;

                // TODO: Necessary?
                if (numBperT == 0)
                {
                    std::cerr << "ERROR: rank distribution calculation does not support cases where a top layer is larger than its bottom layer! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
                    return GTI_ERROR;
                }

                int numRemaining = sB - numBperT * sT; //number of ranks that can't be distributed evenly across the top layer

                int oldIndex = currentTopIndex;
                currentTopIndex = numBperT * currentTopIndex;
                currentTopIndex += std::min (numRemaining, oldIndex);

                oldIndex = currentBottomIndex;
                currentBottomIndex = numBperT * (currentBottomIndex+1);
                currentBottomIndex += std::min (numRemaining, oldIndex+1);
                currentBottomIndex--;
            }
            else if (myDistributions[i] == GTI_BY_BLOCK)
            {
                currentTopIndex = currentTopIndex*myBlocksizes[i];
                currentBottomIndex = (currentBottomIndex+1)*myBlocksizes[i] - 1;
                if (currentBottomIndex >= myLevelSizes[i])
                    currentBottomIndex = myLevelSizes[i] - 1;
            }
        }
        if (outBegin) *outBegin = currentTopIndex;
        if (outEnd) *outEnd = currentBottomIndex;

        return GTI_SUCCESS;
	}

	//=============================
	// getLevelId
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getLevelId (int *outThisLevelId)
	{

	    if (myOwnLevelId < 0)
	    {
	        //Used to initialize own level id
	        getLevelIdForApplicationRank (0, NULL);
	    }
	    if (outThisLevelId)
	        *outThisLevelId = myOwnLevelId;

	    return GTI_SUCCESS;
	}


	//=============================
	// getPlaceDriver (cache)
	//=============================
	template <class T, class Base, bool TLS>
	I_Place * ModuleBase<T, Base, TLS>::myGetPlaceMod ()
	{
#ifdef USE_THREAD_LOCAL
		static thread_local I_Place * placeMod=NULL;
#else
          static sf::contfree_safe_ptr<std::vector<I_Place *>>
              placeMod_vector{};
          int tid = getGtiTid();
          auto x_safe_placeMod_vector = xlock_safe_ptr(placeMod_vector);
          if (x_safe_placeMod_vector->size() < tid + 1)
            x_safe_placeMod_vector->resize(tid + 1, nullptr);
          I_Place *&placeMod = x_safe_placeMod_vector->at(tid);
#endif
                if (!placeMod){
			PNMPI_Service_descriptor_t service = getWrapperService("getPlace", "p");
			int ret = ((int(*)(I_Place**)) service.fct) (&placeMod);
			assert (ret == PNMPI_SUCCESS);
            	}
		return placeMod;
	}
		

	//=============================
	// getPlaceDriver
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getPlaceMod (I_Place** retPlaceMod)
	{
	    *retPlaceMod = myGetPlaceMod();
	    return GTI_SUCCESS;
	}

	//=============================
	// getNodeInLayerId
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getNodeInLayerId (GtiTbonNodeInLayerId* id)
	{
	    if (myOwnLevelId < 0)
	    {
	        //Used to initialize own level id
	        getLevelIdForApplicationRank (0, NULL);
	    }
	    I_Place* PlaceMod = myGetPlaceMod();
	    return PlaceMod->getNodeInLayerId(id);
	}

	//=============================
	// getNumInputChannels
	//=============================
	template <class T, class Base, bool TLS>
	GTI_RETURN ModuleBase<T, Base, TLS>::getNumInputChannels (int *numChannels)
	{
	    //Make sure we loaded our layout information
	    if (myOwnLevelId < 0)
	    {
	        //Not meaningful, but loads the data
	        getLevelIdForApplicationRank (0, NULL);
	    }

	    //Which of the TBON nodes on this layer are we?
	    /**
	     * TODO this is not very clean yet, in the other queries we use an input rank,
	     *           but I made it such that we can avoid that one ... though not clean
	     */
	    DataMapT::iterator pos = myModData.find("id");
	    if (pos == myModData.end())
	        return GTI_ERROR;
	    int ownId = atoi(pos->second.c_str());

	    //Now calculate the minimum and maxium rank we can get events from
	    //We propagate the index level wise
	    int currentTopIndex = ownId;
	    int currentBottomIndex = ownId;

	    if (myOwnLevelId == 0)
	    {
	        if (numChannels)
	            *numChannels = 0;
	    }
	    else
	    {
	        int i = myOwnLevelId - 1;
	        if (myDistributions[i] == GTI_UNIFORM)
	        {
	            int sT= myLevelSizes[i+1];
	            int sB = myLevelSizes[i];

	            int numBperT = sB / sT;

                    // TODO: Necessary?
                    if (numBperT == 0)
                    {
                        std::cerr << "ERROR: rank distribution calculation does not support cases where a top layer is larger than its bottom layer! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
                        return GTI_ERROR;
                    }

	            int numRemaining = sB - numBperT * sT; //number of ranks that can't be distributed evenly across the top layer

	            int oldIndex = currentTopIndex;
	            currentTopIndex = numBperT * currentTopIndex;
	            currentTopIndex += std::min (numRemaining, oldIndex);

	            oldIndex = currentBottomIndex;
	            currentBottomIndex = numBperT * (currentBottomIndex+1);
	            currentBottomIndex += std::min (numRemaining, oldIndex+1);
	            currentBottomIndex--;
	        }
	        else if (myDistributions[i] == GTI_BY_BLOCK)
	        {
	            currentTopIndex = currentTopIndex*myBlocksizes[i];
	            currentBottomIndex = (currentBottomIndex+1)*myBlocksizes[i] - 1;
	            if (currentBottomIndex >= myLevelSizes[i])
	                currentBottomIndex = myLevelSizes[i] - 1;
	        }

	        if (numChannels)
	            *numChannels = currentBottomIndex - currentTopIndex + 1;
	    }

	    return GTI_SUCCESS;
	}

	//=============================
	// getName
	//=============================
	template <class T, class Base, bool TLS>
	std::string ModuleBase<T, Base, TLS>::getName (void)
	{
		return ModuleBase<T, Base, TLS>::ourModName();
	}

} /*namespace gti*/

#endif /*MODULE_BASE_HXX*/
/*EOF*/
