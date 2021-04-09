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
 * @file CProtMpiSplited.cpp
 *       MPI based communication using the split module (CProtMpiSplitModule.cpp).
 *       MPI_COMM_WORLD is split into multiple disjunct sets of processes, set number 0
 *       is used for the application, the remaining sets are used for levels of tool
 *       processes. This protocol establishes a MPI call based communication between
 *       these process sets.
 *
 * @author Tobias Hilbrich
 * @date 28.07.2009
 */

#include <assert.h>
#include <stdio.h>
#include <pnmpimod.h>
#include <algorithm>
#include <mpi.h>
#include "CProtMpiSplited.h"
#include "GtiMacros.h"

using namespace gti;

/*Name of split module @todo move to some common place, currently present twice*/
#define GTI_SPLIT_MODULE_NAME "split_processes"
#define TAG_1 				   666
#define TAG_2 				   777

/**
 * Global variables for MPI state.
 */
static bool gMpiIsInitialized = false;
static bool gMpiIsFinalized = false;

//=============================
// Basic Module functions
//=============================
mGET_INSTANCE_FUNCTION (CProtMPISplited)
mFREE_INSTANCE_FUNCTION (CProtMPISplited)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CProtMPISplited)

//=============================
// initHookNotify
// A helper that notifies all communication protocols
// of type CProtMPISplited of an MPI_Init/MPI_Init_thread
//=============================
extern "C" void initHookNotify (void)
{
	std::map <std::string, CProtMPISplited*> instances = CProtMPISplited::getActiveInstances ();
	std::map <std::string, CProtMPISplited*>::iterator i;

	//save state of MPI
	gMpiIsInitialized = true;

	//notify all existing communication strategies
	for (i = instances.begin(); i != instances.end(); i++)
	{
		CProtMPISplited* prot = i->second;

		if (prot)
			prot->notifyMpiInit();
	}
}

//=============================
// MPI_Init
//=============================
int MPI_Init (int *pArgc, char ***pArgv)
{
	int ret;
	ret = XMPI_Init (pArgc, pArgv);

	//Notify all instances of the init
	initHookNotify ();

	return ret;
}

//=============================
// MPI_Init_thread
//=============================
int MPI_Init_thread (int *pArgc, char ***pArgv, int required, int* provided)
{
	int ret;
	ret = XMPI_Init_thread (pArgc, pArgv, required, provided);

	//Notify all instances of the init
	initHookNotify ();

	return ret;
}

// The protocol is shut down by the wrapper


//=============================
// CProtMPISplitedRequest
//=============================
CProtMPISplitedRequest::CProtMPISplitedRequest (unsigned int id, MPI_Request mpi_request, uint64_t num_bytes, uint64_t channel, bool isRecv)
	: id(id), mpi_request(mpi_request), num_bytes (num_bytes), channel(channel), isRecv(isRecv)
{
	//Nothing else to do
}

//=============================
// CProtMPISplited
//=============================
CProtMPISplited::CProtMPISplited (const char* instanceName)
: ModuleBase<CProtMPISplited, I_CommProtocol> (instanceName),
  myPlaceId (0),
  myNextRequestID (0),
  myComm (MPI_COMM_WORLD),
  myIsConnected (false),
  myWasShutdownCalled (false),
  myRequests (),
  myMapRankToChannel (),
  myPartnerRanks (),
  myIsTop (false),
  myNextRoundRobin(0),
  myNumChannels (0),
  myIsIntra (false)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //Needs no sub modules
    assert (subModInstances.empty());

    /*
     * We now directly call startup here, the below was necessary to enable OTF tracing,
     * however, that wasn't used since a very long time; I think we will remove the OTF
     * tracing stuff in the future anyways.
     */
    //Startup if necessary
    //   If tracable done as part of setupTracing
    //   Otherwise done right now
    //if (!this->isTraceable())
    startup ();
}

//=============================
// ~CProtMPISplited
//=============================
CProtMPISplited::~CProtMPISplited (void)
{
    //nothing to do ...
}

//=============================
// isConnected
//=============================
bool CProtMPISplited::isConnected (void)
{
	return myIsConnected && isInitialized() && !isFinalized();
}

//=============================
// isInitialized
//=============================
bool CProtMPISplited::isInitialized (void)
{
	return gMpiIsInitialized;
}

//=============================
// isFinalized
//=============================
bool CProtMPISplited::isFinalized (void)
{
	return gMpiIsFinalized;
}

//=============================
// notifyMpiInit
//=============================
void CProtMPISplited::notifyMpiInit (void)
{
	if (!myIsConnected && !myWasShutdownCalled)
		startup ();
}

//=============================
// notifyMpiFinalize
//=============================
void CProtMPISplited::notifyMpiFinalize (void)
{
	if (myIsConnected)
		shutdown ();
}

//=============================
// startup
//=============================
GTI_RETURN CProtMPISplited::startup (void)
{
    unsigned int        commId;
    char                commSide;
    int 				err;
    int 				this_set_size,
						other_set_size,
						other_set_start_rank;
    MPI_Comm 			this_set_comm, real_comm_world;

    // === (0) Check whether MPI is initialized
    if (!gMpiIsInitialized)
    {
        return GTI_ERROR_NOT_INITIALIZED;
    }

    // === Check whether already connected
    if (myIsConnected)
        return GTI_SUCCESS;

    char string[512];
    // === (1) Read data ===
    //General protocol data
    std::map<std::string, std::string> data = getData ();
    std::map<std::string, std::string>::iterator i;

    i=data.find("comm_id");
    assert (i != data.end ());
    commId = atoi (i->second.c_str());

    i=data.find("is_intra");
    if (i != data.end())
    {
        if (i->second.c_str()[0] != '0' && i->second.c_str()[0] != '1')
        {
            std::cerr << "Error: Invalid specification for \"is_intra\" module data field in " << __FILE__ << ":" << __LINE__ << std::endl;
            assert (0);
        }

        if (i->second.c_str()[0] == '1')
            myIsIntra = true;
    }

    i=data.find("side");
    assert (myIsIntra || i != data.end ()); //side is not required for intra-communication
    assert (myIsIntra || i->second.length() == 1); //just one letter !
    if (!myIsIntra)
        commSide = i->second.c_str()[0];
    assert (myIsIntra || (commSide == 't') || (commSide == 'b')); //either "t" or "b"

    // === (2) Query for services of the split module ===
    PNMPI_modHandle_t handle;
    PNMPI_Service_descriptor_t service;
    PNMPI_Service_Fct_t fct;

#ifdef PNMPI_FIXED
    err = PNMPI_Service_GetModuleByName(GTI_SPLIT_MODULE_NAME, &handle);
#else
    sprintf (string, "%s",GTI_SPLIT_MODULE_NAME);
    err = PNMPI_Service_GetModuleByName(string, &handle);
#endif
    if (err != PNMPI_SUCCESS)
    {
    	std::cerr
    	<< "Failed to get a handle for the P^nMPI module \""
    	<< GTI_SPLIT_MODULE_NAME
    	<< "\""
    	<< std::endl
    	<< "(Failed in module \""
    	<< getName() << "::" << __FUNCTION__
    	<< "\")"
    	<< std::endl;
    	assert (0);
    }

    err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getMySetSize", "p", &service);
    assert (err == PNMPI_SUCCESS);
    ((int(*)(int*)) service.fct) (&this_set_size);

    err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getMySetComm", "p", &service);
    assert (err == PNMPI_SUCCESS);
    ((int(*)(void*)) service.fct) (&this_set_comm);

    real_comm_world = MPI_COMM_WORLD;
    err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getRealCommWorld", "p", &service);
    if (err == PNMPI_SUCCESS)
    {
        ((int(*)(void*)) service.fct) (&real_comm_world);
    } // else use MPI_COMM_WORLD

    // === (3) Calculate ranks of comm. partner processes ===
    int rank, size;
    int num_partners, remaining, start_index;

    err = PMPI_Comm_rank (this_set_comm, &rank);
    err = PMPI_Comm_size (this_set_comm, &size);
    assert (err == MPI_SUCCESS);
    myPlaceId = rank;

    if (!myIsIntra)
    {
        i=data.find("gti_own_level");
        assert (i != data.end ());
        int top_level, gti_own_level = atoi (i->second.c_str());

        GTI_DISTRIBUTION distrib = GTI_UNIFORM;
        int blocksize;
        err = PNMPI_Service_GetServiceByName(handle, "SplitMod_getSetInfo", "ipp", &service);
        assert (err == PNMPI_SUCCESS);
        ((int(*)(int, int*, int*)) service.fct) (commId,&other_set_size,&other_set_start_rank);

        this_set_size = std::max(this_set_size,1);
        other_set_size = std::max(other_set_size, 1);

        if (commSide == 't')
            top_level = gti_own_level;
        else
            top_level = gti_own_level+1;

        sprintf (string, "gti_level_%i_%i_distribution", top_level - 1 , top_level);
        i=data.find(std::string(string));

        if (i != data.end () and i->second == std::string("by-block"))
        {
            distrib = GTI_BY_BLOCK;
            sprintf (string, "gti_level_%i_%i_blocksize", top_level-1, top_level);
            i=data.find(std::string(string));
            assert(i != data.end ());
            blocksize = atoi(i->second.c_str());
        }

        if (commSide == 't')
        {
            //Top side partner calculation
            myIsTop = true;

            if (distrib == GTI_BY_BLOCK)
            {
                start_index = rank * blocksize;
                num_partners = std::min(blocksize, other_set_size-start_index);
            }
            else
            {
                num_partners = other_set_size / this_set_size;
                remaining = other_set_size - num_partners * this_set_size;

                start_index = /*other_set_start_rank*/0 + rank * num_partners + std::min(remaining,rank);

                if (rank < remaining) //does this rank get one of the remaining processes ?
                    num_partners++;
            }//By-Block or UNIFORM?

            myPartnerRanks.resize(num_partners);
            for (int i = 0; i < num_partners; i++)
            {
                myPartnerRanks[i] = start_index + i; /**< Ranks of comm. partners. */
                myMapRankToChannel.insert (std::make_pair(start_index + i, i));
            }
        }
        else // top or bottom ?
        {
            //bottom side partner calculation
            if (distrib == GTI_BY_BLOCK)
            {
                myPartnerRanks.resize(1);
                myPartnerRanks[0] = rank / blocksize;
                myMapRankToChannel.insert (std::make_pair(myPartnerRanks[0], 0));
            }
            else
            {
                num_partners = this_set_size / other_set_size;
                remaining = this_set_size - num_partners * other_set_size;

                myPartnerRanks.resize(1);
                for (int i = 1; i < other_set_size + 2; i++)
                {
                    if (rank < i * num_partners + std::min(remaining,i))
                    {
                        myPartnerRanks[0] = /*other_set_start_rank*/0 + i - 1;
                        myMapRankToChannel.insert (std::make_pair(myPartnerRanks[0], 0));
                        break;
                    }
                }
            }//By-Block or UNIFORM?
        }// top or bottom ?

        myNumChannels = myPartnerRanks.size();
    }// if !myIsIntra

    //Create the communicator
    if (!myIsIntra)
    {
        //Inter layer communication
        err = PMPI_Intercomm_create (this_set_comm, 0, real_comm_world, other_set_start_rank, TAG_2, &myComm);
    }
    else
    {
        //Intra layer communication
        err = PMPI_Comm_dup (this_set_comm, &myComm);
        myNumChannels = size;
    }
    assert (err == MPI_SUCCESS);

    //DEBUG START
/*
    int myrank;
    err = PMPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    assert (err == MPI_SUCCESS);
    std::cout << "I am rank: " << myrank
    << "My Partners are: {";
    for (int i = 0; i < myPartnerRanks.size(); i++)
    {
    	std::cout << myPartnerRanks[i] << ", ";
    }
    std::cout << "}" << std::endl;
*/
    //DEBUG END

    myIsConnected = true;

    return GTI_SUCCESS;
}

//=============================
// shutdown
//=============================
GTI_RETURN CProtMPISplited::shutdown (void)
{
    myWasShutdownCalled = true;

    //Does a connection exists ?
    if (!myIsConnected)
        return GTI_SUCCESS;

    /*Take care of any outstanding requests*/
    int err = PMPI_Comm_free(&myComm);
    assert (err == MPI_SUCCESS);

    std::map<unsigned int, CProtMPISplitedRequest >::iterator iter;
    for (iter = myRequests.begin(); iter != myRequests.end(); iter++)
    {
        err = PMPI_Cancel(&(iter->second.mpi_request));
        assert (err == MPI_SUCCESS);
        err = PMPI_Request_free(&(iter->second.mpi_request));
        assert (err == MPI_SUCCESS);
    }
    myRequests.clear();

    //Set as unconnected
    myIsConnected = false;
    return GTI_SUCCESS;
}

//=============================
// removeOutstandingRequests
//=============================
GTI_RETURN CProtMPISplited::removeOutstandingRequests (void)
{
	int err;

	if (!myIsConnected)
		return GTI_SUCCESS;

	/*TODO: This is MPI-2, we need to ifdef here for an MPI-1 alternative.*/
#ifndef GTI_NO_MPI2
	err = PMPI_Barrier (myComm);
	assert (err == MPI_SUCCESS);
#else
	if (myIsIntra)
	{
	    err = PMPI_Barrier (myComm);
	}
	else
	{
        for (int i = 0; i < myNumChannels; i++)
        {
            MPI_Status status;
            if (myIsTop)
                PMPI_Recv (NULL, 0, MPI_INT, getRankForChannel(i), 12345, myComm, &status);
            else
                PMPI_Ssend (NULL, 0, MPI_INT, getRankForChannel(i), 12345, myComm);
        }
	}
#endif

	std::map<unsigned int, CProtMPISplitedRequest >::iterator iter;
	for (iter = myRequests.begin(); iter != myRequests.end(); iter++)
	{
	    err = PMPI_Cancel(&(iter->second.mpi_request));
	    assert (err == MPI_SUCCESS);
	    err = PMPI_Request_free(&(iter->second.mpi_request));
	    assert (err == MPI_SUCCESS);
	}
	myRequests.clear();

    return GTI_SUCCESS;
}

//=============================
// getNumChannels
//=============================
GTI_RETURN CProtMPISplited::getNumChannels (
        uint64_t* out_numChannels)
{
	if (!myIsConnected)
		return GTI_ERROR_NOT_INITIALIZED;

    if (out_numChannels)
        *out_numChannels = myNumChannels;

    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CProtMPISplited::getPlaceId (uint64_t* outPlaceId)
{
    if (!myIsConnected)
            return GTI_ERROR_NOT_INITIALIZED;

    if (outPlaceId)
        *outPlaceId = myPlaceId;

    return GTI_SUCCESS;
}

//=============================
// ssend
//=============================
GTI_RETURN CProtMPISplited::ssend (
        void* buf,
        uint64_t num_bytes,
        uint64_t channel
        )
{
	int err;

	if (!myIsConnected)
		return GTI_ERROR_NOT_INITIALIZED;

    //Sanity
    assert (channel < myNumChannels);

    //Send!
    err = PMPI_Ssend (buf, num_bytes, MPI_BYTE, getRankForChannel(channel), TAG_1, myComm);
    assert (err == MPI_SUCCESS);

    return GTI_SUCCESS;
}

//=============================
// isend
//=============================
GTI_RETURN CProtMPISplited::isend (
        void* buf,
        uint64_t num_bytes,
        unsigned int *out_request,
        uint64_t channel
        )
{
	if (!myIsConnected)
		return GTI_ERROR_NOT_INITIALIZED;

    MPI_Request mpi_request;
    int err;

    //Sanity
    assert (channel < myNumChannels);

    //ISend
    /**
     * DO NOT REMOVE THIS!
     * While calling an MPI_Issend from time to time must look like the most stupid
     * of ideas, it is crucial! Mvapich on Sierra has a weird progress implementation.
     * It will acknowledge MPI_Isend requests in MPI_Wait with successfull, but then
     * create a huge internal list of requests that it goes through for every wait,
     * test, and irecv/isend call. If we purely use Isend Strategies in MUST,
     * having this silly piece of code in here makes a performance impact of a factor
     * 60 and more!
     *
     * Oh, and the 13 comes from testing :D Value of 5 is too low, 20 too high in
     * the test I used, I guess impact depends on particular test case at hand.
     */
    static uint64_t iteration = 0;
    if (iteration % 50 == 49)
        err = PMPI_Issend (buf, num_bytes, MPI_BYTE, getRankForChannel(channel), TAG_1, myComm, &mpi_request);
    else
        err = PMPI_Isend (buf, num_bytes, MPI_BYTE, getRankForChannel(channel), TAG_1, myComm, &mpi_request);
    iteration++;
    assert (err == MPI_SUCCESS);

    //Store and return request (map MPI request to an int)
    myRequests.insert (std::make_pair(
        	myNextRequestID,
        	CProtMPISplitedRequest(myNextRequestID, mpi_request, num_bytes, channel, false)));
    *out_request = myNextRequestID;
    myNextRequestID++;

    return GTI_SUCCESS;
}

//=============================
// recv
//=============================
GTI_RETURN CProtMPISplited::recv (
        void* out_buf,
        uint64_t num_bytes,
        uint64_t* out_length,
        uint64_t channel,
        uint64_t *out_channel
        )
{
	if (!myIsConnected)
		return GTI_ERROR_NOT_INITIALIZED;

    MPI_Status status;
    int err;

    if (channel != RECV_ANY_CHANNEL)
        assert (channel < myNumChannels);

    //do the receive
    if (channel == RECV_ANY_CHANNEL)
    {
    		//To avoid starvation of some ranks in a scenario where the receiver side is a bottleneck and
    	    	//the connected processes overflood this process, we use an MPI_IProbe in a round robin fashion
    	    	//to avoid always receiving from the same rank
    	    	MPI_Status probeStatus;
    	    	int probeFlag;
    	    	err = PMPI_Iprobe (getRankForChannel(myNextRoundRobin), TAG_1, myComm, &probeFlag, &probeStatus);

    	    	int sourceToUse = MPI_ANY_SOURCE;
   	    	if (probeFlag)
   	    		sourceToUse = getRankForChannel(myNextRoundRobin);
   	    	myNextRoundRobin=(myNextRoundRobin+1)%myNumChannels;

    		err = PMPI_Recv (out_buf, num_bytes, MPI_BYTE, sourceToUse, TAG_1, myComm, &status);
		assert (err == MPI_SUCCESS);
    }
    else
    {
        err = PMPI_Recv (out_buf, num_bytes, MPI_BYTE, getRankForChannel(channel), TAG_1, myComm, &status);
        assert (err == MPI_SUCCESS);
    }

    //save the used channel and the message length
    uint64_t used_channel = channel;
    if (used_channel == RECV_ANY_CHANNEL)
    {
		used_channel = getChannelForRank(status.MPI_SOURCE);
    }

    if (out_channel)
        *out_channel = used_channel;

    int count;
    err = PMPI_Get_count (&status, MPI_BYTE, &count);
    assert (err == MPI_SUCCESS);

    if (out_length)
        *out_length = count;

    return GTI_SUCCESS;
}

//=============================
// irecv
//=============================
GTI_RETURN CProtMPISplited::irecv (
        void* out_buf,
        uint64_t num_bytes,
        unsigned int *out_request,
        uint64_t channel
        )
{
	if (!myIsConnected)
		return GTI_ERROR_NOT_INITIALIZED;

    MPI_Request request;
    int err;

    if (channel != RECV_ANY_CHANNEL)
    	assert (channel < myNumChannels);

    //do the receive
    if (channel == RECV_ANY_CHANNEL)
    {
    		//To avoid starvation of some ranks in a scenario where the receiver side is a bottleneck and
    	    //the connected processes overfood this process, we use an MPI_IProbe in a round robin fashion
    	    //to avoid always receiving from the same rank
    	    MPI_Status probeStatus;
    	    int probeFlag;
    	    err = PMPI_Iprobe (getRankForChannel(myNextRoundRobin), TAG_1, myComm, &probeFlag, &probeStatus);

    	    int sourceToUse = MPI_ANY_SOURCE;
    	    if (probeFlag)
    	    		sourceToUse = getRankForChannel (myNextRoundRobin);
    	    myNextRoundRobin=(myNextRoundRobin+1)%myNumChannels;

		err = PMPI_Irecv (out_buf, num_bytes, MPI_BYTE, sourceToUse, TAG_1, myComm, &request);
		assert (err == MPI_SUCCESS);
    }
    else
    {
        err = PMPI_Irecv (out_buf, num_bytes, MPI_BYTE, getRankForChannel(channel), TAG_1, myComm, &request);
        assert (err == MPI_SUCCESS);
    }

    //Store and return request (map MPI request to an int)
    myRequests.insert (std::make_pair(
            	myNextRequestID,
            	CProtMPISplitedRequest(myNextRequestID, request, num_bytes, channel, true)));

    if (out_request)
        *out_request = myNextRequestID;
    myNextRequestID++;

    return GTI_SUCCESS;
}

//=============================
// test_msg
//=============================
GTI_RETURN CProtMPISplited::test_msg (
        unsigned int request,
        int* out_completed,
        uint64_t* out_receive_length,
        uint64_t* out_channel
        )
{
    if (out_completed)
        *out_completed = 0;

	if (!myIsConnected)
		return GTI_ERROR_NOT_INITIALIZED;

    int err;
    std::map<unsigned int, CProtMPISplitedRequest >::iterator iter;
    iter = myRequests.find(request);

	assert (iter != myRequests.end()); //has to be valid

    MPI_Status status;
    int flag;
    err = PMPI_Test (&(iter->second.mpi_request), &flag, &status);
    assert (err == MPI_SUCCESS);

    if (flag)
    {
        if (out_completed)
            *out_completed = 1;

        //save the used channel and the message length
        int count;
        uint64_t channel_used;

        if (iter->second.isRecv)
        {
            channel_used = getChannelForRank(status.MPI_SOURCE);

            err = PMPI_Get_count (&status, MPI_BYTE, &count);
            assert (err == MPI_SUCCESS);
        }
        else
        {
            channel_used = iter->second.channel;
            count = iter->second.num_bytes;
        }

        if (out_channel)
            *out_channel = channel_used;

        if (out_receive_length)
            *out_receive_length = count;

        myRequests.erase (iter);
    }

    return GTI_SUCCESS;
}

//=============================
// wait_msg
//=============================
GTI_RETURN CProtMPISplited::wait_msg (
        unsigned int request,
        uint64_t* out_receive_length,
        uint64_t* out_channel
        )
{
	if (!myIsConnected)
		return GTI_ERROR_NOT_INITIALIZED;

    std::map<unsigned int, CProtMPISplitedRequest >::iterator iter;
    iter = myRequests.find(request);

    assert (iter != myRequests.end()); //has to be valid

    MPI_Status status;
    int err;

    err = PMPI_Wait (&(iter->second.mpi_request), &status);
    assert (err == MPI_SUCCESS);

   	//save the used channel and the message length
    int count;
    uint64_t channel_used;

    if (iter->second.isRecv)
    {
        channel_used = getChannelForRank(status.MPI_SOURCE);

        err = PMPI_Get_count (&status, MPI_BYTE, &count);
        assert (err == MPI_SUCCESS);
    }
    else
    {
        channel_used = iter->second.channel;
        count = iter->second.num_bytes;
    }

    if (out_channel)
        *out_channel = channel_used;

    if (out_receive_length)
        *out_receive_length = count;


    myRequests.erase (iter);

    return GTI_SUCCESS;
}

//=============================
// getRankForChannel
//=============================
int CProtMPISplited::getRankForChannel (int channel)
{
    if (myIsIntra)
        return channel;

    return myPartnerRanks[channel];
}

//=============================
// getChannelForRank
//=============================
int CProtMPISplited::getChannelForRank (int rank)
{
    if (myIsIntra)
        return rank;

    return myMapRankToChannel[rank];
}

/*EOF*/
