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
 * @file BufferChecks.cpp
 *       @see MUST::BufferChecks.
 *
 *  @date 11.01.2013
 *  @author Joachim Protze, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "BufferChecks.h"
#include "MustEnums.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(BufferChecks)
mFREE_INSTANCE_FUNCTION(BufferChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(BufferChecks)

//=============================
// Constructor
//=============================
BufferChecks::BufferChecks (const char* instanceName)
    : gti::ModuleBase<BufferChecks, I_BufferChecks> (instanceName), bufferSize(-1), bufferLoad(0)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBMODULES 3
    if (subModInstances.size() < NUM_SUBMODULES)
    {
            std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
            assert (0);
    }
    if (subModInstances.size() > NUM_SUBMODULES)
    {
            for (std::vector<I_Module*>::size_type i = NUM_SUBMODULES; i < subModInstances.size(); i++)
                destroySubModuleInstance (subModInstances[i]);
    }

    myPIdMod = (I_ParallelIdAnalysis*) subModInstances[0];
    myLogger = (I_CreateMessage*) subModInstances[1];
    myArgMod = (I_ArgumentAnalysis*) subModInstances[2];

    //Initialize module data
    //Nothing to do
}

//=============================
// Destructor
//=============================
BufferChecks::~BufferChecks ()
{
    if (myPIdMod)
        destroySubModuleInstance ((I_Module*) myPIdMod);
    myPIdMod = NULL;

    if (myLogger)
        destroySubModuleInstance ((I_Module*) myLogger);
    myLogger = NULL;

    if (myArgMod)
        destroySubModuleInstance ((I_Module*) myArgMod);
    myArgMod = NULL;

}

//=============================
// bufferAttach
//=============================
GTI_ANALYSIS_RETURN BufferChecks::bufferAttach (
        MustParallelId pId, 
        MustLocationId lId, 
        int aId, 
        int size)
{
    if (bufferSize!=-1)
    {
        std::stringstream stream;
        stream
            <<  "There was already a buffer attached. Only one buffer can be attached to a process at a time.";

        myLogger->createMessage(
                MUST_ERROR_BUFFER_REATTACH,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    bufferSize = size;
    bufferLoad = 0;
    return GTI_ANALYSIS_SUCCESS;
}

GTI_ANALYSIS_RETURN BufferChecks::bufferDetach (
        MustParallelId pId, 
        MustLocationId lId)
{
    if (bufferSize == -1)
    {
        std::stringstream stream;
        stream
            <<  "There was no buffer attached previously.";

        myLogger->createMessage(
                MUST_ERROR_BUFFER_NOATTACHED,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    bufferSize = -1;
    return GTI_ANALYSIS_SUCCESS;
}

GTI_ANALYSIS_RETURN BufferChecks::bufferUsage (
        MustParallelId pId, 
        MustLocationId lId,
        int size)
{
    if (bufferSize == -1)
    {
        std::stringstream stream;
        stream
            <<  "There is no MPI buffer attached. Using buffering send operations while no buffer is attached is an error. You need to call MPI_Buffer_attach before this call.";

        myLogger->createMessage(
                MUST_ERROR_BUFFER_NOATTACHED,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    if (bufferSize - bufferLoad < size)
    {
        std::stringstream stream;
        stream
            <<  "Latest buffer usage of " << size << " bytes outsized the attached buffer's size of "<< bufferSize <<" bytes while "<< bufferLoad <<" bytes are already in use!" << std::endl
            <<  "This may be a false positive warning as the only implemented method to detect the completion of a Bsend operation is a call to MPI_Buffer_detach." << std::endl
            <<  "To make use of this check you may want to call MPI_Buffer_detach(buf,&size) + MPI_Buffer_attach(buf,size) whenever you expect all pending Bsends to be finished.";

        myLogger->createMessage(
                MUST_WARNING_BUFFER_OUTSIZED,
                pId,
                lId,
                MustWarningMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    bufferLoad += size;
    return GTI_ANALYSIS_SUCCESS;
}


/*EOF*/
