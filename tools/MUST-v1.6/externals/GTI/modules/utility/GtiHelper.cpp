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
 * @file GtiHelper.cpp
 *       @see gti::GtiHelper
 *
 * @author Felix M�nchhalfen
 *
 * @date 16.04.2014
 *
 */

#include "GtiHelper.h"

#include <mpi.h>
#include <pnmpimod.h>
#include <string>
#include <iostream>

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "pnmpi-config.h"
#ifdef ELP_MODIFICATIONS
       #include <unistd.h>
       #include <sys/syscall.h>
    #ifdef _OPENMP
        #include <omp.h>
    #endif
#endif

using namespace gti;

pthread_mutex_t GtiHelper::myIdLock;
bool GtiHelper::myInitedRank = false;
int GtiHelper::myRankInLayer = 0;

GtiHelper::GtiHelperStaticInitializer initializer;

GtiHelper::GtiHelperStaticInitializer::GtiHelperStaticInitializer()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&myIdLock, &attr);
}

GtiHelper::GtiHelperStaticInitializer::~GtiHelperStaticInitializer()
{
    pthread_mutex_destroy(&myIdLock);
}

//=============================
// GtiHelper
//=============================
GtiHelper::GtiHelper (void)
{
    /*Nothing to do*/
}

//=============================
// ~GtiHelper
//=============================
GtiHelper::~GtiHelper (void)
{
    /*Nothing to do*/
}

//=============================
// getFakedCommWorld
//=============================
inline MPI_Comm getFakedCommWorld()
{
    static MPI_Comm fakeCommWorld = MPI_COMM_NULL;
    static int inited,
    got_comm = 0;

    if (!got_comm) {
        //get rank of this process
        PMPI_Initialized(&inited);

        if (!inited)
            return fakeCommWorld; //TODO: what id to use in such a case ?

        int err;
        PNMPI_modHandle_t handle;
        PNMPI_Service_descriptor_t service;

        //We need to check whether MPI_COMM_WORLD was splited
#ifdef PNMPI_FIXED
        err = PNMPI_Service_GetModuleByName("split_processes", &handle);
#else
        char string[512];
        sprintf(string, "%s", "split_processes");
        err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
        if (err == PNMPI_SUCCESS) {
            err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getMySetComm", "p", &service);
            assert(err == PNMPI_SUCCESS);
            ((int(*)(void*)) service.fct) (&fakeCommWorld);

        }
        else {
            //No splitting is active, use MPI_COMM_WORLD
            fakeCommWorld = MPI_COMM_WORLD;
        }
        got_comm = 1;
    }

    return fakeCommWorld;
}

//=============================
// buildLayer
//=============================
GtiTbonNodeInLayerId GtiHelper::buildLayer(bool threaded)
{
#ifdef ELP_MODIFICATIONS
    if( threaded )
        return buildLayerIdAsHybridLayer();
    else
        return buildLayerIdAsPureMpiLayer();
#else
    return buildLayerIdAsPureMpiLayer();
#endif
}

//=============================
// buildLayerIdAsPureMpiLayer
//=============================
GtiTbonNodeInLayerId GtiHelper::buildLayerIdAsPureMpiLayer()
{
    int rank = 0;

#ifdef ELP_MODIFICATIONS
    pthread_mutex_lock(&myIdLock);
#endif
    if (!myInitedRank)
    {
        MPI_Comm comm = getFakedCommWorld();

        if (comm != MPI_COMM_NULL)
            PMPI_Comm_rank(comm, &rank);

        myRankInLayer = rank;
        myInitedRank = true;
    }
#ifdef ELP_MODIFICATIONS
    pthread_mutex_unlock(&myIdLock);
#endif

    return myRankInLayer;
}

//=============================
// buildLayerIdAsHybridLayer
//=============================
GtiTbonNodeInLayerId GtiHelper::buildLayerIdAsHybridLayer()
{
    int threadid = 0;
    
    GtiTbonNodeInLayerId myId = 0;
    int myRank = buildLayerIdAsPureMpiLayer();
    
    //Determine the a unique thread-id (TODO: Which method to use?)
    #if defined(SYS_gettid) && defined(ELP_MODIFICATIONS)
    threadid = syscall(SYS_gettid);
    #elif defined(_OPENMP) && defined(ELP_MODIFICATIONS)
    threadid = omp_get_thread_num();
    #elif defined(ELP_MODIFICATIONS)
    threadid = pthread_self(); // Probably the most portable one!
    #endif

    // Use the upper 32bits for the thread id, lower 32bit for the MPI rank
    myId = ((GtiTbonNodeInLayerId)threadid << 32) | myRank;

    return myId;
}

//=============================
// getInstanceName
//=============================
GTI_RETURN GtiHelper::getInstanceName (const char **instanceName)
{
	//get self module, get name of own instance
	int err = PNMPI_SUCCESS;
	PNMPI_modHandle_t modHandle;

	err=PNMPI_Service_GetModuleSelf(&modHandle);
	assert (err == PNMPI_SUCCESS);

	/* get own module name */
	char temp[64];
	sprintf (temp, "instanceToUse");
	err=PNMPI_Service_GetArgument(modHandle, temp, instanceName);

	if (err != PNMPI_SUCCESS)
	{
		std::cerr << "Error: tool place module needs a PnMPI argument named \"instanceToUse\" that lists a valid instance name to be used as instance." << std::endl;
		return GTI_ERROR;
	}

	return GTI_SUCCESS;
}

/*EOF*/
