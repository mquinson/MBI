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
 * @file DatatypeChecks.cpp
 *       @see MUST::DatatypeChecks.
 *
 *  @date 23.05.2011
 *  @author Joachim Protze, Tobias Hilbrich
 */

#include "GtiMacros.h"
#include "DatatypeChecks.h"
#include "MustEnums.h"

#include <sstream>

using namespace must;

mGET_INSTANCE_FUNCTION(DatatypeChecks)
mFREE_INSTANCE_FUNCTION(DatatypeChecks)
mPNMPI_REGISTRATIONPOINT_FUNCTION(DatatypeChecks)

//=============================
// Constructor
//=============================
DatatypeChecks::DatatypeChecks (const char* instanceName)
    : gti::ModuleBase<DatatypeChecks, I_DatatypeChecks> (instanceName)
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUBMODULES 4
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
    myDatMod = (I_DatatypeTrack*) subModInstances[3];

    //Initialize module data
    //Nothing to do
}

//=============================
// Destructor
//=============================
DatatypeChecks::~DatatypeChecks ()
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

    if (myDatMod)
        destroySubModuleInstance ((I_Module*) myDatMod);
    myDatMod = NULL;
}

//=============================
// errorIfNotKnown
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNotKnown (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType datatype)
{
    I_Datatype * info = myDatMod->getDatatype(pId, datatype);
    return DatatypeChecks::errorIfNotKnown(pId, lId, aId, info);
}

GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNotKnown (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    I_Datatype * info, 
    int index)
{
    if (info==NULL)
    {
        std::stringstream stream;
        if (index==-1)
        {
        stream
            <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
            << ") is an unknown datatype (neither a predefined nor a user defined datatype)!";
        }
        else
        {
            stream
                << "Element of Array-Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << "[" << index << "]) is an unknown datatype (neither a predefined nor a user defined datatype)!";
        }

        myLogger->createMessage(
                MUST_ERROR_DATATYPE_UNKNOWN,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNull
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNull (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType datatype)
{
    I_Datatype * info = myDatMod->getDatatype(pId, datatype);
    if (info == NULL)
        return GTI_ANALYSIS_SUCCESS;
    return DatatypeChecks::errorIfNull(pId, lId, aId, info);
}

GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNull (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    I_Datatype * info,
    int index)
{
    if (info == NULL)
        return GTI_ANALYSIS_SUCCESS;

    if (info->isNull())
    {
        std::stringstream stream;
        if (index==-1)
        {
            stream
                <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << ") is MPI_DATATYPE_NULL!";
        }
        else
        {
            stream
                << "Element of Array-Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << "[" << index << "]) is MPI_DATATYPE_NULL!";
        }

        myLogger->createMessage(
                MUST_ERROR_DATATYPE_NULL,
                pId,
                lId,
                MustErrorMessage,
                stream.str()
                );
        return GTI_ANALYSIS_FAILURE;
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNotCommited
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNotCommited (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType datatype)
{
    I_Datatype * info = myDatMod->getDatatype(pId, datatype);
    if (info == NULL)
        return GTI_ANALYSIS_SUCCESS;
    return DatatypeChecks::errorIfNotCommited(pId, lId, aId, info, datatype);
}

GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNotCommited (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    I_Datatype * info,
    MustDatatypeType& datatype,
    int index)
{
    if (info == NULL)
        return GTI_ANALYSIS_SUCCESS;
    if (!info->isPredefined() && !info->isCommited())
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;
        if (index==-1)
        {
            stream
                <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << ") is not commited for transfer, call MPI_Type_commit before using the type for transfer!" << std::endl 
                << "(Information on " << myArgMod->getArgName(aId);
            info->printInfo (stream, &refs);
            stream << ")" << std::endl;
        }
        else
        {
            stream
                << "Element of Array-Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << "[" << index << "]) is not commited for transfer, call MPI_Type_commit before using the type for transfer!" << std::endl 
                << "(Information on " << myArgMod->getArgName(aId);
            info->printInfo (stream, &refs);
            stream << ")" << std::endl;
        }

        myLogger->createMessage(
                MUST_ERROR_DATATYPE_NOT_COMMITED,
                pId,
                lId,
                MustErrorMessage,
                stream.str(),
                refs
                );
        return GTI_ANALYSIS_FAILURE;
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// warningIfCommited
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::warningIfCommited (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType datatype)
{
    I_Datatype * info = myDatMod->getDatatype(pId, datatype);
    if (info == NULL)
        return GTI_ANALYSIS_SUCCESS;
    return DatatypeChecks::warningIfCommited(pId, lId, aId, info, datatype);
}

GTI_ANALYSIS_RETURN DatatypeChecks::warningIfCommited (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    I_Datatype * info,
    MustDatatypeType& datatype,
    int index)
{
    if (info == NULL)
        return GTI_ANALYSIS_SUCCESS;
    if (info->isPredefined())
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;
        if (index==-1)
        {
            stream
                <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << ") is predefined, there is no need to commit it!" << std::endl 
                << "(Information on " << myArgMod->getArgName(aId);
            info->printInfo (stream, &refs);
            stream << ")" << std::endl;
        }
        else
        {
            stream
                << "Element of Array-Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << "[" << index << "]) is predefined, there is no need to commit it!" << std::endl 
                << "(Information on " << myArgMod->getArgName(aId);
            info->printInfo (stream, &refs);
            stream << ")" << std::endl;
        }

        myLogger->createMessage(
                MUST_WARNING_DATATYPE_PREDEFINED,
                pId,
                lId,
                MustWarningMessage,
                stream.str(),
                refs
                );
        return GTI_ANALYSIS_FAILURE;
    }
    if (info->isCommited())
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;
        if (index==-1)
        {
            stream
                <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << ") is already commited, there is no need to commit it again!" << std::endl 
                << "(Information on " << myArgMod->getArgName(aId);
            info->printInfo (stream, &refs);
            stream << ")" << std::endl;
        }
        else
        {
            stream
                << "Element of Array-Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
                << "[" << index << "]) is already commited, there is no need to commit it again!" << std::endl 
                << "(Information on " << myArgMod->getArgName(aId);
            info->printInfo (stream, &refs);
            stream << ")" << std::endl;
        }

        myLogger->createMessage(
                MUST_WARNING_DATATYPE_COMMITED,
                pId,
                lId,
                MustWarningMessage,
                stream.str(),
                refs
                );
        return GTI_ANALYSIS_FAILURE;
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// warningIfNotPropperlyAligned
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::warningIfNotPropperlyAligned (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType datatype)
{
    I_Datatype * info = myDatMod->getDatatype(pId, datatype);
    if (info == NULL)
        return GTI_ANALYSIS_SUCCESS;
    return DatatypeChecks::warningIfNotPropperlyAligned(pId, lId, aId, info, datatype);
}

GTI_ANALYSIS_RETURN DatatypeChecks::warningIfNotPropperlyAligned (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    I_Datatype * info,
    MustDatatypeType& datatype,
    int index)
{
    if (info == NULL)
        return GTI_ANALYSIS_SUCCESS;
    if (info->getTypesig().size()<2)
    { // only one base type
        return GTI_ANALYSIS_SUCCESS;
    }
    MustAddressType pos;
    if ((pos = info->checkAlignment()))
    {
        std::list<std::pair<MustParallelId,MustLocationId> > refs;
        std::stringstream stream;
        stream
            <<  "Argument " << myArgMod->getIndex(aId) << " (" << myArgMod->getArgName(aId)
            << ") has an unusual alignment; this is typically a sign that the type does " 
            << "not match with application struct!" << std::endl 
            << "(Information on " << myArgMod->getArgName(aId);
        info->printInfo (stream, &refs);
        stream << ")" << std::endl;
    
        myLogger->createMessage(
                MUST_WARNING_DATATYPE_BAD_ALIGNMENT,
                pId,
                lId,
                MustWarningMessage,
                stream.str(),
                refs
                );
        return GTI_ANALYSIS_FAILURE;
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNotValidForCommunication
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNotValidForCommunication (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType datatype)
{
    I_Datatype * info = myDatMod->getDatatype(pId, datatype);
    
    return (errorIfNotKnown(pId, lId, aId, info)!=GTI_ANALYSIS_SUCCESS || 
            errorIfNull(pId, lId, aId, info)!=GTI_ANALYSIS_SUCCESS || 
            errorIfNotCommited(pId, lId, aId, info, datatype)!=GTI_ANALYSIS_SUCCESS) ? GTI_ANALYSIS_FAILURE : GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfArrayNotValidForCommunication
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::errorIfArrayNotValidForCommunication (
    MustParallelId pId,
    MustLocationId lId,
    int aId,
    MustDatatypeType* datatypes,
    int count)
{
    I_Datatype * info;
    int i;
    for (i=0; i<count; i++)
    {
        info = myDatMod->getDatatype(pId, datatypes[i]);
        if (errorIfNotKnown(pId, lId, aId, info, i)!=GTI_ANALYSIS_SUCCESS ||
                errorIfNull(pId, lId, aId, info, i)!=GTI_ANALYSIS_SUCCESS ||
         errorIfNotCommited(pId, lId, aId, info, datatypes[i], i)!=GTI_ANALYSIS_SUCCESS)
        {
            return GTI_ANALYSIS_FAILURE;
        }
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNotValidForCreate
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNotValidForCreate (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType datatype)
{
    I_Datatype * info = myDatMod->getDatatype(pId, datatype);
    return (errorIfNotKnown(pId, lId, aId, info)!=GTI_ANALYSIS_SUCCESS || 
            errorIfNull(pId, lId, aId, info)!=GTI_ANALYSIS_SUCCESS) ? GTI_ANALYSIS_FAILURE : GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfArrayNotValidForCreate
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::errorIfArrayNotValidForCreate (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType* datatypes,
    int count)
{
    I_Datatype * info;
    int i;
    for (i=0; i<count; i++)
    {
        info = myDatMod->getDatatype(pId, datatypes[i]);
        if (errorIfNotKnown(pId, lId, aId, info, i)!=GTI_ANALYSIS_SUCCESS ||
                errorIfNull(pId, lId, aId, info, i)!=GTI_ANALYSIS_SUCCESS)
        {
            return GTI_ANALYSIS_FAILURE;
        }
    }
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// errorIfNotValidForCommit
//=============================
GTI_ANALYSIS_RETURN DatatypeChecks::errorIfNotValidForCommit (
    MustParallelId pId, 
    MustLocationId lId, 
    int aId, 
    MustDatatypeType datatype)
{
    I_Datatype * info = myDatMod->getDatatype(pId, datatype);
    return (errorIfNotKnown(pId, lId, aId, info)!=GTI_ANALYSIS_SUCCESS || 
            errorIfNull(pId, lId, aId, info)!=GTI_ANALYSIS_SUCCESS || 
            warningIfCommited(pId, lId, aId, info, datatype)!=GTI_ANALYSIS_SUCCESS) ? GTI_ANALYSIS_FAILURE : GTI_ANALYSIS_SUCCESS;
}


/*EOF*/
