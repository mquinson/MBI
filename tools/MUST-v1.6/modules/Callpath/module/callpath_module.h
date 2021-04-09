/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */


#ifndef PNMPI_MOD_CALLPATH
#define PNMPI_MOD_CALLPATH

///\file callpath_module.h
///\author Todd Gamblin, tgamblin@llnl.gov
///
///=== Overview ==========================================================
/// This file defines the interface for the PnMPI callpath module.
///
/// The callpath module allows an application to take a call trace at
/// the top of the PnMPI stack and make it accessible to all modules 
/// below that point as a service.  For example, suppose you had the 
/// following PnMPI module stack:
///
/// +---------------+
/// |  Application  |
/// +---------------+
/// |   MPI Call    |
/// +===============+  <-- Application/PnMPI boundary  
/// |   callpath    |  <-- Stack is traced here
/// +---------------+
/// |   module 1    |  <-+
/// +---------------+    |
/// |   module 2    |  <-+-- All modules can access callpath trace
/// +---------------+    |   of only MPI and frames and above.
/// |   module 3    |  <-+   
/// +---------------+
///
/// Using this module, tools can avoid extra overhead and confusing data.
/// There are three reasons to use this:
///   1. Taking the call trace once for every module avoids extra
///      stackwalker overhead, which can be expensive depending on the 
///      platform.
///
///   2. Taking the call trace at the top of the PnMPI stack avoids
///      including PnMPI frames in the call trace.  There are potentially
///      large numbers of PnMPI frames, and these can confuse some
///      measurement tools.   They can also cost extra overhead to walk
///      through.
///
///   3. Tool modules can refer to the same set of callpaths to describe
///      the application.  If each tool takes its own trace, callpaths
///      are not directly comparable, as each tool gets a different 
///      callpath because of its depth in the PnMPI stack.
///
///=== Requirements ======================================================
/// This module requires the callpath library, by Todd Gamblin.
/// You can download it here:
/// 
///     https://github.com/tgamblin/callpath
///
/// This module and the callpath library are intended to be used from C++.
/// 
///=== Usage =============================================================
/// 
/// To use this module, You will need to query first for the callpath module,
/// then for the PNMPIMOD_Callpath_GetCallpath function.  Here is boilerplate
/// code to get everything set up:
///
///  <code>
///    #include "callpath_module.h"
///
///    PNMPIMOD_Callpath_GetCallpath_t get_callpath;
/// 
///    // find the callpath module
///    PNMPI_modHandle_t module;
///    int err = PNMPI_Service_GetModuleByName(PNMPI_MODULE_CALLPATH, &module);
///    if (err!=PNMPI_SUCCESS) {
///      cerr << "Couldn't find module " PNMPI_MODULE_CALLPATH << endl;
///      return err;
///    }
///
///    // get the service that will get us the actual call trace
///    PNMPI_Service_descriptor_t service;
///    err = PNMPI_Service_GetServiceByName(module, PNMPIMOD_Callpath_GetCallpath, "r", &service);
///    if (err != PNMPI_SUCCESS) {
///      cerr << "Couldn't find " PNMPIMOD_Callpath_GetCallpath " service!" << endl;
///      return err;
///    }
///    get_callpath = reinterpret_cast<PNMPIMOD_Callpath_GetCallpath_t>(service.fct);
///  </code>
///
/// After you've successfully gotten the service function, you can call it to
/// get the Callpath from the callpath library, e.g.:
/// <code>
///   Callpath my_callpath = get_callpath();
/// </code>
#include "Callpath.h"

#include <list>

/// String key for querying PnMPI whether this module is available.
#define PNMPI_MODULE_CALLPATH "pnmpi-module-callpath"

struct StackInfo
{
    std::string symName;
    std::string lineOffset; //Line if available offset otherwise
    std::string fileModule; //File name if available, module otherwise
};

/// Type of the GetCallpath function that this module provides.
typedef std::list<StackInfo> (*PNMPIMOD_Callpath_GetCallpath_t)();

/// Query name of the GetCallpath function so that it can be looked up
/// using PNMPI_Service_GetServiceByName()
#define PNMPIMOD_Callpath_GetCallpath "PNMPIMOD_Callpath_GetCallpath"


#endif // PNMPI_MOD_CALLPATH
