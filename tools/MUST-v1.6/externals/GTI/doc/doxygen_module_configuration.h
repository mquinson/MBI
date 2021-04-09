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
 * @page ModConfPage Details on Module Configuration at Start-Up
 *
 * @section ModConfOverview Overview
 *
 * For GTI each PnMPI module represents one C++ class. As classes can have
 * multiple instances it is allowed that multiple instances
 * of a module are used. This page handles construction, destruction,
 * retrieval, cooperation, and configuration of modules.
 *
 * Each module needs information about:
 * - The instances to use as its sub modules
 * - Its data in the form of key+value pairs, e.g. to specify an IP address
 *   for a TCP based communication module
 *
 * Each module should use the following three function to enable the
 * construction and destruction of its modules:
 * - PNMPI_RegistrationPoint reads a modules argument (syntax and contents see below)
 *      to read which instances of the module will be used and what their configuration is.
 *      Must register the module with the name that is given in its module argument.
 *      Must register the getInstance and freeInstance functions below.
 * - int getInstance (void *ppInstance, void* instanceName) returns an instance. The argument
 *      ppInstance is of type I_Module** -- void* is used for PnMPI registration -- and
 *      is used to store the pointer to the created module instance in. The argument
 *      instanceName is of type char* -- again void* used for PnMPI registration -- and
 *      gives the name of the requested module instance. If an unknown name is given
 *      (not read from the PnMPI configuration during PNMPI_RegistrationPoint) no instance
 *      shall be returned. Returns PNMPI_SUCCESS if successful.
 * - int freeInstance (void* pInstance) frees the given instance. The argument pInstance
 *      is of type I_Module* -- void* is used for PnMPI registration.
 *
 * Special registered services are:
 * - int addDataHandler (const void* pInstanceName, const void* pKey, const void* pValue)
 *   used by gti::ModuleBase to implement gti::ModuleBase::addDataToSubmodules.
 *   pInstanceName, pKey, and pValue have (const char*) as actual type.
 * - int getFunction (const char* functionName, GTI_Fct_t** pOutFunctionAddr) which needs
 *   to be implemented by all wrapper modules. It is used by modules that need to call
 *   wrapper functions to get the function pointer to call. This is also reflected
 *   in the module configuration where "instance<N>Wrapper" is used to specify the name
 *   of the wrapper to use.
 *
 * IMPORTANT:
 *  The names used to register these functions to PnMPI need to eactly match the above names,
 *  though the names of the symbols for these functions must differ for each pair
 *  of modules. I.e. no two modules may use the name for their implementation of
 *  getInstance. This may be done by prepending the name of the module class to the name
 *  of these functions. Also no two modules may use the same class name. This
 *  also needs to be keept in mind when generating modules.
 *  The issue here is that if this is not done the wrong functions may be called which
 *  may lead to very ugly bugs.
 *
 * A module instance may be used by multiple other modules. The first call to getInstance will allocate
 * and initialize the instance, whereas subsequent calls will return a pointer to the existing instance.
 * Each user has to free the instance with freeInstance, a reference count mechanism must be used
 * to deallocate the instance when the last freeInstance call arrives.
 *
 * For common examples of these functions see: GtiMacros.h
 *
 * See gti::ModuleBase for a base class that implements all the functionality needed
 * to read sub modules and module data as well as to support reference counting and tracking
 * of existing instances. Each module should inherit from this class to gain a simple
 * and correct implementation for these individual functionalities. The three above
 * functions that work along with gti::ModuleBase are found in GtiMacros.h.
 *
 * @section ModConf Module Configuration Syntax
 *
 * The information on the instances of a module, their sub modules and data is specified as
 * follows in the PnMPI configuration:
 * @verbatim
 module <NAME_OF_MODULE_LIB>
 argument moduleName <NAME_OF_MODULE>
 argument numInstances <N>
 argument instance0 <INSTANCE_NAME>
 ...
 argument instanceN-1 <INSTANCE__NAME>
 argument instance0SubMods <SUB_MOD_SPEC>
 ...
 argument instanceN-1SubMods <SUB_MOD_SPEC>
 argument instance0Data <DATA_SPEC>
 ...
 argument instanceN-1Data <DATA_SPEC>
 argument instance0Wrapper <MOD_NAME>
 ...
 argument instanceN-1Wrapper <MOD_NAME>
 @endverbatim
 *
 * The indivdual entries have the following meaning:
 * - <NAME_OF_MODULE_LIB> is the library name of the module
 * - <NAME_OF_MODULE> is the name given to this module
 *                    must be used in PNMPI_RegistrationPoint to
 *                    register the module. Other module instances
 *                    will use this name to refer to this module.
 * - <N> Number of instances used of this module.
 * - <INSTANCE_NAME> name of an instance
 * - <SUB_MOD_SPEC> specification of sub modules of an instance. Is a list of
 * 					comma separated ModuleName+Instance pairs of the form:
 *                  <MODULE_NAME>:<INSTANCE_NAME>, e.g. "a:b,c:d"
 * - <DATA_SPEC> specification for data passed to an instance. Is a list of
 *               comma separated Key+Value pairs of the form <KEY>=<VALUE>,
 *               e.g.: "ip=localhost,port=5555"
 * - The definition of "instance<N>Wrapper" is optional and used for modules
 *   that need to call a function in a wrapper. I.e. to implement the wrapp-everywhere
 *   functionality that functions may have. gti::ModuleBase::getWrapperFunction
 *   retrieves symbols from such a wrapper specification. <MOD_NAME> is a single
 *   module name.
 *
 *
 * Example:
 * @verbatim
 module libtestModConf
 argument moduleName ModName1
 argument numInstances 3
 argument instance0 instA
 argument instance1 instB
 argument instance2 instC
 argument instance0SubMods ModName1:instC,ModName1:instB
 argument instance1SubMods ModName1:instC
 argument instance0Data key1=value1,key2=value2
 argument instance1Data key3=value3
 argument instance2Data key4=value4,key5=value5
  @endverbatim
 *
 * Special modules may use further arguments, or even not work as a GTI module at all.
 * Examples are:
 * - ThreadedMPIPlace -- uses "instanceToUse" to denote which instance to use for running the place
 * - cprot_mpi_cplit_module.cxx -- a purely PnMPI type module used to split MPI_COMM_WORLD uses arguments to influence the splitting
 */
