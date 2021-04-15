/* This file is part of MUST (Marmot Umpire Scalable Tool)
 *
 * Copyright (C)
 *  2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
 *  2010-2018 Lawrence Livermore National Laboratories, United States of America
 *  2013-2018 RWTH Aachen University, Federal Republic of Germany
 *
 * See the LICENSE file in the package base directory for details
 */

#include <cstring>
#include "callpath_module.h"
#include "FrameInfo.h"
#include "FrameId.h"
#include "Translator.h"

#include <pnmpi/hooks.h>
#include <pnmpi/service.h>
#include "GtiTLS.h"

using namespace std;

/// This is the actual callpath that's written by the wrappers.
thread_local Callpath PNMPIMOD_Callpath_callpath;

/// This function just returns the callpath.  
/// Wrappers are responsible for setting it.
std::list<StackInfo> PNMPIMOD_Callpath_GetCallpath_Fct() {
    std::list<StackInfo> ret;

//    Callpath slice= PNMPIMOD_Callpath_callpath.slice(3);
     Callpath &slice= PNMPIMOD_Callpath_callpath;

    static thread_local Translator myTrans = Translator();
    FrameInfo info;
    for(int i = slice.size()-1; i >= 0; i--)
    {
        info = myTrans.translate(slice.get(i));

        //Is it an MPI function, if so stop
        if ((info.sym_name[0] == 'M' || info.sym_name[0] == 'm') &&
            (info.sym_name[1] == 'P' || info.sym_name[1] == 'p') &&
            (info.sym_name[2] == 'I' || info.sym_name[2] == 'i') &&
            info.sym_name[3] == '_')
            break;

	//Is it an PnMPI function, if so stop
        if ((info.sym_name[0] == 'N' || info.sym_name[0] == 'n') &&
            (info.sym_name[1] == 'Q' || info.sym_name[1] == 'q') &&
            (info.sym_name[2] == 'J' || info.sym_name[2] == 'j') &&
            info.sym_name[3] == '_')
            break;

        //Add to list
        StackInfo sInfo;
        sInfo.symName = info.sym_name;
        if (info.file != "")
            sInfo.fileModule = info.file;
        else
            sInfo.fileModule = info.module.c_str();
        if (info.line_num != "")
            sInfo.lineOffset = info.line_num;
        else
            sInfo.lineOffset = info.offset;

        ret.push_front(sInfo);
    }

    return ret;
}


void PNMPI_RegistrationPoint() {
  // register this module
  int err;
  err = PNMPI_Service_RegisterModule(PNMPI_MODULE_CALLPATH);
  assert (err == PNMPI_SUCCESS);

  // register its services
  PNMPI_Service_descriptor_t service;
  sprintf(service.name, PNMPIMOD_Callpath_GetCallpath);
  service.fct = (PNMPI_Service_Fct_t) PNMPIMOD_Callpath_GetCallpath_Fct;
  sprintf(service.sig, "r");
  err = PNMPI_Service_RegisterService(&service);
  assert(err == PNMPI_SUCCESS);
}
