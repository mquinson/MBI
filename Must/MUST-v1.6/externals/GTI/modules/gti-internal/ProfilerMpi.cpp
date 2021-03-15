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
 * @file ProfilerMpi.cpp
 *       @see MUST::ProfilerMpi.
 *
 *  @date 16.07.2012
 *  @author Tobias Hilbrich, Joachim Protze, Mathias Korepkat
 */

#include "GtiMacros.h"
#include "GtiApi.h"

#include "ProfilerMpi.h"

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(ProfilerMpi);
mFREE_INSTANCE_FUNCTION(ProfilerMpi);
mPNMPI_REGISTRATIONPOINT_FUNCTION(ProfilerMpi);

#define GTI_SPLIT_MODULE_NAME "split_processes"

//=============================
// Helper
//=============================
inline uint64_t getUsecTime ()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000000 + t.tv_usec;
}

//=============================
// MPI_Finalize
//=============================
int MPI_Finalize (void)
{
    //notify all existing profilers
    std::map <std::string, ProfilerMpi*> instances = ProfilerMpi::getActiveInstances ();
    std::map <std::string, ProfilerMpi*>::iterator i;

    for (i = instances.begin(); i != instances.end(); i++)
    {
        ProfilerMpi* p = i->second;
        if (p)
            p->report();
    }

    return PMPI_Finalize ();
}

//=============================
// Constructor
//=============================
ProfilerMpi::ProfilerMpi (const char* instanceName)
    : gti::ModuleBase<ProfilerMpi, I_Profiler> (instanceName),
      myStartTime (0),
      myIdleTime (0),
      myLastWrapperEntryTime (0),
      myMaxBadness (0),
      myDownTime (),
      myUpTime (),
      myIntraTime (),
      myAnalysisTimes ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_SUB_MODS 0
/*    if (subModInstances.size() < NUM_SUB_MODS)
    {
            std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
            assert (0);
    }*/
    if (subModInstances.size() > NUM_SUB_MODS)
    {
            for (int i = NUM_SUB_MODS; i < subModInstances.size(); i++)
                destroySubModuleInstance (subModInstances[i]);
    }

    //Initialize module data
    myStartTime = getUsecTime ();
}

//=============================
// Destructor
//=============================
ProfilerMpi::~ProfilerMpi ()
{
    report ();
}

//=============================
// reportIdleTime
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::reportIdleTime (uint64_t usecIdle)
{
    myIdleTime += usecIdle;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// reportMaxBadness
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::reportMaxBadness (uint64_t badness)
{
    myMaxBadness += badness;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// reportTimeoutTime
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::reportTimeoutTime (uint64_t usec, uint64_t count)
{
    myTimeoutTime.first += usec;
    myTimeoutTime.second += count;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// reportDownCommTime
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::reportDownCommTime (uint64_t usecDown, uint64_t count)
{
    myDownTime.first += usecDown;
    myDownTime.second += count;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// reportUpCommTime
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::reportUpCommTime (uint64_t usecUp, uint64_t count)
{
    myUpTime.first += usecUp;
    myUpTime.second += count;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// reportIntraCommTime
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::reportIntraCommTime (uint64_t usecIntra, uint64_t count)
{
    myIntraTime.first += usecIntra;
    myIntraTime.second += count;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// reportWrapperAnalysisTime
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::reportWrapperAnalysisTime (std::string moduleName, std::string analysisName, uint64_t time, uint64_t count)
{
    myAnalysisTimes[moduleName][analysisName].first.first = time;
    myAnalysisTimes[moduleName][analysisName].first.second = count;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// reportReceivalAnalysisTime
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::reportReceivalAnalysisTime (std::string moduleName, std::string analysisName, uint64_t time, uint64_t count)
{
    myAnalysisTimes[moduleName][analysisName].second.first = time;
    myAnalysisTimes[moduleName][analysisName].second.second = count;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// setWrapperEntryTime
//=============================
GTI_ANALYSIS_RETURN ProfilerMpi::setWrapperEntryTime (uint64_t usecTimeStamp)
{
    myLastWrapperEntryTime = usecTimeStamp;
    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// getLastWrapperEntryTime
//=============================
uint64_t ProfilerMpi::getLastWrapperEntryTime (void)
{
    return myLastWrapperEntryTime;
}

//=============================
// report
//=============================
void ProfilerMpi::report (void)
{
    static bool wasReported = false;
    if (wasReported) return;
    wasReported = true;

    //== 1) Final calculations
    uint64_t endTime = getUsecTime();

    uint64_t totalAnalysisTime = 0;
    uint64_t totalAnalysisInvocations = 0;
    int numFunctions = 0;
    ModuleMap::iterator modIter;
    FunctionMap::iterator fIter;
    for (modIter = myAnalysisTimes.begin(); modIter != myAnalysisTimes.end(); modIter++)
    {
        for (fIter = modIter->second.begin(); fIter != modIter->second.end(); fIter++)
        {
            numFunctions++;

            //Only add to total averages if the invocation count is > 0; We have some special counts (queue sizes and the like) that we store with invocation counts of 0
            if (fIter->second.first.second)
            {
                totalAnalysisTime += fIter->second.first.first;
                totalAnalysisInvocations += fIter->second.first.second;
            }

            //Only add to total averages if the invocation count is > 0; We have some special counts (queue sizes and the like) that we store with invocation counts of 0
            if (fIter->second.second.second)
            {
                totalAnalysisTime += fIter->second.second.first;
                totalAnalysisInvocations += fIter->second.second.second;
            }
        }
    }

    myInfrastructureTime = endTime - myStartTime - myIdleTime - myDownTime.first - myUpTime.first - myIntraTime.first - myTimeoutTime.first - totalAnalysisTime;

    //== 2) Catch the right layer communicator, or use MPI comm world
    MPI_Comm comm = MPI_COMM_WORLD;
    int rank, size;

    PNMPI_modHandle_t handle;
    PNMPI_Service_descriptor_t service;
    PNMPI_Service_Fct_t fct;
    int err;

#ifdef PNMPI_FIXED
    err = PNMPI_Service_GetModuleByName(GTI_SPLIT_MODULE_NAME, &handle);
#else
    char string[256];
    sprintf (string, "%s",GTI_SPLIT_MODULE_NAME);
    err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
    if (err == PNMPI_SUCCESS)
    {
        err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getMySetComm", "p", &service);
        assert (err == PNMPI_SUCCESS);
        ((int(*)(void*)) service.fct) (&comm);
    }

    PMPI_Comm_rank (comm, &rank);
    PMPI_Comm_size (comm, &size);

    //== 3) Prepare times and counts into array
    /*
     * Format (4 + 4 + numFunctions*2) * 2 values
     * Content:
     *  - myIdleTime, 0
     *  - myMaxBadness, 0
     *  - myInfrastructureTime, 0
     *  - myDownTime
     *  - myUpTime
     *  - myIntraTime
     *  - myTimeoutTime
     *  - totalAnalysisTime, totalAnalysisInvocations
     *  - <individual analysis function times>
     */
#define IDX_IDLE 0
#define IDX_BAD 2
#define IDX_INFRA 4
#define IDX_DOWN 6
#define IDX_UP 8
#define IDX_INTRA  10
#define IDX_TOUT 12
#define IDX_AN 14

    int nodeInfoSize = 8*2 + numFunctions*4;
    uint64_t* sendData = new uint64_t[nodeInfoSize];
    sendData[IDX_IDLE] = myIdleTime;
    sendData[IDX_IDLE+1] = 0;
    sendData[IDX_BAD] = myMaxBadness;
    sendData[IDX_BAD+1] = 0;
    sendData[IDX_INFRA] = myInfrastructureTime;
    sendData[IDX_INFRA+1] = 0;
    sendData[IDX_DOWN] = myDownTime.first;
    sendData[IDX_DOWN+1] = myDownTime.second;
    sendData[IDX_UP] = myUpTime.first;
    sendData[IDX_UP+1] = myUpTime.second;
    sendData[IDX_INTRA] = myIntraTime.first;
    sendData[IDX_INTRA+1] = myIntraTime.second;
    sendData[IDX_TOUT] = myTimeoutTime.first;
    sendData[IDX_TOUT+1] = myTimeoutTime.second;
    sendData[IDX_AN] = totalAnalysisTime;
    sendData[IDX_AN+1] = totalAnalysisInvocations;

    int index = IDX_AN+2;
    for (modIter = myAnalysisTimes.begin(); modIter != myAnalysisTimes.end(); modIter++)
    {
        for (fIter = modIter->second.begin(); fIter != modIter->second.end(); fIter++, index+=4)
        {
            sendData[index] = fIter->second.first.first;
            sendData[index+1] = fIter->second.first.second;
            sendData[index+2] = fIter->second.second.first;
            sendData[index+3] = fIter->second.second.second;
        }
    }

    //== 4) Gather all data of this layer
    uint64_t *receiveData = NULL;
    MPI_Datatype type = MPI_UNSIGNED_LONG;
    if (sizeof(uint64_t) != sizeof(uint64_t))
        type = MPI_UNSIGNED_LONG_LONG;

    if (rank == 0)
        receiveData = new uint64_t[nodeInfoSize*size];

    PMPI_Gather (sendData, nodeInfoSize, type, receiveData, nodeInfoSize, type, 0, comm);

    //== 5) Write the result
    if (rank == 0)
    {
        //= open output
        std::stringstream outNameStream;
        outNameStream << "gti_layer_" << this->getData()["gti_own_level"] << ".profile";
        std::ofstream out (outNameStream.str().c_str());

        //= Layout information
        out << "layer-information" << std::endl;
        std::map<std::string,std::string> modData = getData();
        if (modData.find("gti_own_level") != modData.end())
        {
            int ownLevel = atoi (modData["gti_own_level"].c_str());
            out << "index " << ownLevel << std::endl;

            for (int i = 0; i <= ownLevel; i++)
            {
                //Read level size
                std::stringstream keyName;
                keyName << "gti_level_" << i << "_size";
                if (modData.find(keyName.str()) == modData.end())
                    continue;

                out << "levelSize_" << i << " " << atoi (modData[keyName.str()].c_str()) << std::endl;

                if (i == ownLevel)
                    continue;

                std::stringstream distribName;
                GTI_DISTRIBUTION distrib = GTI_UNIFORM;
                distribName << "gti_level_" << i << "_" << i+1 << "_distribution";
                if (modData.find(distribName.str()) == modData.end())
                    continue;

                out << "levelDistribution_" << i << "_" << i+1 << " ";

                if (modData.find(distribName.str())->second == "by-block")
                {
                        out << "by-block" << std::endl;

                        //Also read the blocksize
                        std::stringstream bsizeName;
                        bsizeName << "gti_level_" << i << "_" << i+1 << "_blocksize";
                        if (modData.find(bsizeName.str()) == modData.end())
                            continue;

                        out << "levelBlocksize_" << i << "_" << i+1 << " " <<  atoi (modData.find(bsizeName.str())->second.c_str()) << std::endl;
                }
                else if (modData.find(distribName.str())->second == "uniform")
                {
                    out << "uniform" << std::endl;
                }
            }//for levels
        }//own level id present
        out << std::endl;

        //= Basic times
        //total time
        out << "totalTime\t";
        for (int i = 0; i < size; i++)
        {
            out
            << receiveData[IDX_IDLE+i*nodeInfoSize] +
            receiveData[IDX_INFRA+i*nodeInfoSize] +
            receiveData[IDX_DOWN+i*nodeInfoSize] +
            receiveData[IDX_UP+i*nodeInfoSize] +
            receiveData[IDX_INTRA+i*nodeInfoSize] +
            receiveData[IDX_TOUT+i*nodeInfoSize] +
            receiveData[IDX_AN+i*nodeInfoSize] << "\t0\t";
        }
        out << std::endl;

        //idle time
        out << "idleTime\t";
        for (int i = 0; i < size; i++)
            out << receiveData[IDX_IDLE+i*nodeInfoSize] << "\t0\t";
        out << std::endl;

        //max badness
        out << "maxFloodBadness\t";
        for (int i = 0; i < size; i++)
            out << receiveData[IDX_BAD+i*nodeInfoSize] << "\t0\t";
        out << std::endl;

        //infra time
        out << "infrastructureTime\t";
        for (int i = 0; i < size; i++)
            out << receiveData[IDX_INFRA+i*nodeInfoSize] << "\t0\t";
        out << std::endl;

        //down time
        out << "downTime\t";
        for (int i = 0; i < size; i++)
            out << receiveData[IDX_DOWN+i*nodeInfoSize] << "\t" << receiveData[IDX_DOWN+i*nodeInfoSize+1] <<  "\t";
        out << std::endl;

        //up time
        out << "upTime\t";
        for (int i = 0; i < size; i++)
            out << receiveData[IDX_UP+i*nodeInfoSize] << "\t" << receiveData[IDX_UP+i*nodeInfoSize+1] <<  "\t";
        out << std::endl;

        //intra time
        out << "intraTime\t";
        for (int i = 0; i < size; i++)
            out << receiveData[IDX_INTRA+i*nodeInfoSize] << "\t" << receiveData[IDX_INTRA+i*nodeInfoSize+1] <<  "\t";
        out << std::endl;

        //timeout time
        out << "timeoutTime\t";
        for (int i = 0; i < size; i++)
            out << receiveData[IDX_TOUT+i*nodeInfoSize] << "\t" << receiveData[IDX_TOUT+i*nodeInfoSize+1] <<  "\t";
        out << std::endl;

        //total analysis time
        out << "analysisTime\t";
        for (int i = 0; i < size; i++)
            out << receiveData[IDX_AN+i*nodeInfoSize] << "\t" << receiveData[IDX_AN+i*nodeInfoSize+1] <<  "\t";
        out << std::endl;

        out << std::endl;

        //= Break downs per module
        int index = IDX_AN + 2;
        for (modIter = myAnalysisTimes.begin(); modIter != myAnalysisTimes.end(); modIter++)
        {
            out << "module " << modIter->first << std::endl;
            out << "numFunctions " << modIter->second.size() << std::endl;

            for (fIter = modIter->second.begin(); fIter != modIter->second.end(); fIter++, index+=4)
            {
                if (fIter->first != "")
                    out << "wraperFunction " << fIter->first << "\t";
                else
                    out << "wraperFunction unnamedFunction" << "\t";

                for (int i = 0; i < size; i++)
                    out << receiveData[index+i*nodeInfoSize] << "\t" << receiveData[index+i*nodeInfoSize+1] <<  "\t";
                out << std::endl;

                if (fIter->first != "")
                    out << "receivalFunction " << fIter->first << "\t";
                else
                    out << "receivalFunction unnamedFunction" << "\t";

                for (int i = 0; i < size; i++)
                    out << receiveData[index+i*nodeInfoSize+2] << "\t" << receiveData[index+i*nodeInfoSize+3] <<  "\t";
                out << std::endl;
            }

            out << std::endl;
        }
    }

    //==6) Clean up
    if (sendData) delete [] sendData;
    if (receiveData) delete [] receiveData;
}

/*EOF*/
