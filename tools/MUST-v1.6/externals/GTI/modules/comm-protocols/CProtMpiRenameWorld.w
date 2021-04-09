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
 * @file CProtMpiSplitModule.cpp generated from cprot_mpi_split_module.w
 *       Splits MPI processes into multiple sets of processes.
 *       Intention is to use one set(id:0) for the actual application
 *       and the remaining sets for tool processes (one set for
 *       each level of the tool).
 *       Further, a separate stack is used for each of the sets,
 *       to enable tools with distinct layouts. Also, the actual
 *       application calls are separated such that MPI_COMM_WORLD
 *       is replaced by a comm representing the application
 *       processes set.
 *       For all the tool process sets only an MPI_Init and an
 *       MPI_Finalize is called, afterwards an exit(0) is issued,
 *       so all tool startup should be done in MPI_Init and it should
 *       only return once a shutdown is desired.
 *
 * @author Tobias Hilbrich
 * @date 27.07.2009
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <sstream>

#include <dlfcn.h>

#include <pnmpimod.h>
#include "GtiMacros.h"

/**
 * Global definitions and helpers.
 */
/*Name of split module @todo move to some common place, currently present twice*/
#define MUST_SPLIT_MODULE_NAME "replace_module"
MPI_Comm fakeComm;
#define MACRO_MPI_Comm(_c) \
{ if (_c == MPI_COMM_WORLD) _c = fakeComm; }

/**
 * PNMPI_ReistrationPoint
 */
extern "C" void PNMPI_RegistrationPoint()
{
  int err;

  /* register this module*/
  err=PNMPI_Service_RegisterModule(MUST_SPLIT_MODULE_NAME);
  assert (err == PNMPI_SUCCESS);
}

#ifdef MUST_TIME
struct timeval gStart, gEnd;
#endif

/* extern "C" int MPI_Init(int * argc, char *** argv)
 { */
{{fn fn_name MPI_Init MPI_Init_thread}} {
  int err;
  err = {{callfn}}
  assert (err == MPI_SUCCESS);
    PNMPI_modHandle_t handle;
    PNMPI_Service_descriptor_t service;
    PNMPI_Service_Fct_t fct;

    //We need to check whether MPI_COMM_WORLD was splited
#ifdef PNMPI_FIXED
    err = PNMPI_Service_GetModuleByName("split_processes", &handle);
#else
    char string[512];
    sprintf (string, "%s","split_processes");
    err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
    if (err == PNMPI_SUCCESS)
    {

        err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getMySetComm", "p", &service);
        assert (err == PNMPI_SUCCESS);
        ((int(*)(void*)) service.fct) (&fakeComm);

    }
} {{endfn}}

/*
 * -------------------------------------------
 */
/*GENERATED FUNCTIONS*/
/*
 * -------------------------------------------
 */
/* automatically generated wrapper code */
/*
 * MPI_Attr_get and MPI_Attr_put are in this list since an MPI_Attr_get on a replaced MPI_COMM_WORLD may not provide
 * the default attributes that are attached with MPI_COMM_WORLD. Since we also do not wrap the MPI_Attr_put this
 * should be fine.
 */

{{fnall fn_name MPI_Init MPI_Init_thread MPI_Pcontrol MPI_Finalize MPI_Wtime MPI_Wticks  MPI_Attr_get MPI_Comm_get_attr MPI_Attr_put MPI_Comm_put_attr}} {
  {{apply_to_type MPI_Comm MACRO_MPI_Comm}}
WRAP_MPI_CALL_PREFIX
  {{ret_val}} = P{{fn_name}}({{args}});
WRAP_MPI_CALL_POSTFIX
}{{endfnall}}
