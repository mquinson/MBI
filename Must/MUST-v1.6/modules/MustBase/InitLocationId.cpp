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
 * @file InitLocationId.cpp
 *       @see must::InitLocationId.
 *
 *  @date 24.04.2014
 *  @author Tobias Hilbrich
 */

#include "GtiMacros.h"
#include <pnmpi.h>
#include <assert.h>
#include "MustDefines.h"

#include "InitLocationId.h"

#ifdef USE_CALLPATH
#include "pnmpimod.h"
#include "callpath_module.h"
#endif

using namespace must;

mGET_INSTANCE_FUNCTION(InitLocationId)
mFREE_INSTANCE_FUNCTION(InitLocationId)
mPNMPI_REGISTRATIONPOINT_FUNCTION(InitLocationId)

//=============================
// Constructor
//=============================
InitLocationId::InitLocationId (const char* instanceName)
    : gti::ModuleBase<InitLocationId, I_InitLocationId> (instanceName),
      myKnownLocations ()
{
    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //handle sub modules
#define NUM_MODS_REQUIRED 1
    if (subModInstances.size() < NUM_MODS_REQUIRED)
    {
        std::cerr << "Module has not enough sub modules, check its analysis specification! (" << __FILE__ << "@" << __LINE__ << ")" << std::endl;
        assert (0);
    }
    if (subModInstances.size() > NUM_MODS_REQUIRED)
    {
        for (std::vector<I_Module*>::size_type i = NUM_MODS_REQUIRED; i < subModInstances.size(); i++)
            destroySubModuleInstance (subModInstances[i]);
    }

    myPIdInit = (I_InitParallelId*) subModInstances[0];

    //Module data
    getWrapperFunction ("handleNewLocation", (GTI_Fct_t*)&myNewLocFct);

    if (!myNewLocFct)
    {
        std::cerr << "InitLocationId module could not find the \"handleNewLocation\" function and will not operate correctly as a result. Check the module mappings and specifications for this module and the function. Aborting." << std::endl;
        assert (0);
    }
}

//=============================
// init
//=============================
GTI_ANALYSIS_RETURN InitLocationId::init (MustLocationId *pStorage, const char* callName, int callId)
{
    if (!pStorage)
        return GTI_ANALYSIS_FAILURE;

    MustLocationId id; //result value
    uint32_t occCount = 0; //occurrence count of this callId

#ifdef USE_CALLPATH
    //a) For the callpath case: get the service function from the callpath module
    static PNMPIMOD_Callpath_GetCallpath_t fct;
    static bool isInitialized = false;

    if (!isInitialized)
    {
        isInitialized = true;

        // find the callpath module
        PNMPI_modHandle_t module;
        int err = PNMPI_Service_GetModuleByName(PNMPI_MODULE_CALLPATH, &module);
        if (err!=PNMPI_SUCCESS) {
            std::cerr << "Couldn't find module " PNMPI_MODULE_CALLPATH << std::endl;
            return GTI_ANALYSIS_FAILURE;
        }

        // get the service that will get us the actual call trace
        PNMPI_Service_descriptor_t service;
        err = PNMPI_Service_GetServiceByName(module, PNMPIMOD_Callpath_GetCallpath, "r", &service);
        if (err != PNMPI_SUCCESS) {
            std::cerr << "Couldn't find " PNMPIMOD_Callpath_GetCallpath " service!" << std::endl;
            return GTI_ANALYSIS_FAILURE;
        }

        fct = reinterpret_cast<PNMPIMOD_Callpath_GetCallpath_t>(service.fct);
    }

    //b) Build the information for this location (callName + stack)
    std::list<StackInfo> sList = (*fct) ();
    LocationInfo thisLocation;
    thisLocation.callName = callName;

    std::list<StackInfo>::iterator sIter;
    for(sIter = sList.begin(); sIter != sList.end(); sIter++)
    {
        MustStackLevelInfo levelInfo;
        levelInfo.symName = sIter->symName;
        levelInfo.fileModule = sIter->fileModule;
        levelInfo.lineOffset = sIter->lineOffset;
        thisLocation.stack.push_back(levelInfo);
    }

    //c) Search in the known locations
    static int nextLocationId = 0;
    KnownLocationsType::iterator pos;

    pos = myKnownLocations.find (callId);
    if (pos == myKnownLocations.end ())
    {
        //c-1) Its a new location
        occCount = 1;
        std::map<LocationInfo, MustLocationId> temp;
        temp.insert (std::make_pair(thisLocation, nextLocationId));
        myKnownLocations.insert (std::make_pair (callId, std::make_pair (temp, occCount)));
        createHandleNewLocationCall (nextLocationId, (char*)(void*)callName, thisLocation);
        nextLocationId++;
        id = nextLocationId-1;
    }
    else
    {
        //c-2) We have used this call id already (either new or old)
        std::map<LocationInfo, MustLocationId>::iterator callIdPos;

        callIdPos = pos->second.first.find (thisLocation);
        pos->second.second = pos->second.second + 1;
        occCount = pos->second.second;

        if (callIdPos == pos->second.first.end())
        {
            //A new stack
            pos->second.first.insert (std::make_pair(thisLocation, nextLocationId));
            createHandleNewLocationCall (nextLocationId, (char*)(void*)callName, thisLocation);
            nextLocationId++;
            id = nextLocationId-1;
        }
        else
        {
            //A known stack
            id = callIdPos->second;
        }
    }
#else
    //Search in the known locations
    KnownLocationsType::iterator pos;
    id = callId;
    pos = myKnownLocations.find (callId);
    if (pos == myKnownLocations.end ())
    {
        //Its a new location
        occCount = 1;
        LocationInfo info;
        info.callName = callName;
        myKnownLocations.insert (std::make_pair (id, std::make_pair(info, occCount)));

        // Note: we cast the const away from the callName, but the function should only have reading access
        MustParallelId pId;
        myPIdInit->init(&pId);
        (*myNewLocFct) (pId, id, (char*)(void*)callName, info.callName.length()+1);
    }
    else
    {
        pos->second.second = pos->second.second + 1;
        occCount = pos->second.second;
    }
#endif

    //Store it
    //Lower 32 bit represent the location identifier, upper 32bit represent occurrence count
    *pStorage = (id & 0x00000000FFFFFFFF) | ((uint64_t)occCount << 32);

    return GTI_ANALYSIS_SUCCESS;
}

//=============================
// createHandleNewLocationCall
//=============================
#ifdef USE_CALLPATH
void InitLocationId::createHandleNewLocationCall (MustLocationId id, char* callName, LocationInfo &location)
{
       char totalInfo[MUST_MAX_TOTAL_INFO_SIZE];
       int InfoIndices[MUST_MAX_NUM_STACKLEVELS*3];
       int maxtotalLen = MUST_MAX_TOTAL_INFO_SIZE -  MUST_MAX_NUM_STACKLEVELS*4;
       int totalLength = 0;
       int infoIndicesIndex = 0;

       std::list<MustStackLevelInfo>::iterator iter;
       for (iter = location.stack.begin(); iter != location.stack.end() && infoIndicesIndex<MUST_MAX_NUM_STACKLEVELS*3; iter++)
       {
           for (int piece = 0; piece < 3; piece++)
           {
               const char* info = NULL;

               switch (piece)
               {
               case 0: info = iter->symName.c_str();
                   break;
               case 1: info = iter->fileModule.c_str();
                   break;
               case 2: info = iter->lineOffset.c_str();
                   break;
               }

               int i = 0;
               while (info && info[i] != '\0' && totalLength < maxtotalLen)
               {
                   totalInfo[totalLength] = info[i];
                   i++;
                   totalLength++;
               }
               totalInfo[totalLength] = '\0';
               totalLength++;

               InfoIndices[infoIndicesIndex] = totalLength-1;
               infoIndicesIndex++;
           }
       }

       MustParallelId pId;
       myPIdInit->init(&pId);
       (*myNewLocFct) (
               pId,
               id,
               callName,
               location.callName.length()+1,
               infoIndicesIndex/3, /*Num stack levels*/
               totalLength, /*stack infos total length*/
               infoIndicesIndex, /*indicesLength*/
               InfoIndices, /*infoIndices*/
               totalInfo); /*StackInfos*/
}
#endif


/*EOF*/
