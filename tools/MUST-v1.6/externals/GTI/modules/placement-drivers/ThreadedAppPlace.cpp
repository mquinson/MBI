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
 * @file ThreadedAppPlace.cpp
 *       A placement driver that is spawned as a thread in an
 *       application process.
 *
 *
 * @author Tobias Hilbrich, Felix Münchhalven, Joachim Protze
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <inttypes.h>

#include <pnmpimod.h>
#include "ThreadedAppPlace.h"
#include "GtiMacros.h"
#include "SuspensionBufferTree.h"

#ifdef GTI_VT
#include <vt_user.h>

void* theHandle = 0;

typedef void (*startP) (const char* name, const char* file, int lno);
typedef void (*endP) (const char* name);

void VT_User_start__(const char* name, const char* file, int lno)
{
	if (!theHandle)
		theHandle = dlopen ("libvt-mpi.so", RTLD_LAZY);

	startP fp = (startP) dlsym (theHandle, "VT_User_start__");
	fp (name, file, lno);
}

void VT_User_end__(const char* name)
{
	endP fp = (endP) dlsym (theHandle, "VT_User_end__");
	fp (name);
}
#endif /*GTI_VT*/


using namespace gti;

mGET_INSTANCE_FUNCTION (ThreadedAppPlace)
mFREE_INSTANCE_FUNCTION (ThreadedAppPlace)
mPNMPI_REGISTRATIONPOINT_FUNCTION(ThreadedAppPlace)

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
// Helper to create the master module for this place
// and issue its "run" function.
//=============================
extern "C" int MPI_Init_thread (int *pArgc, char ***pArgv, int required, int* provided)
{
    //get name of instance to use
    const char *instanceName;
    GTI_RETURN ret = GtiHelper::getInstanceName (&instanceName);
    assert (ret == GTI_SUCCESS);

    //Initialize and run place
    ThreadedAppPlace* place;
    place = ThreadedAppPlace::getInstance(instanceName);
    assert(place);
    place->init();

    int res = XMPI_Init_thread(pArgc, pArgv, required, provided);

    place->run();
    ThreadedAppPlace::freeInstanceForced(place);
    return res;
}

//=============================
// MPI_Finalize Wrapper
//=============================
#if 0
extern "C" int MPI_Finalize (void)
{
    static bool wasHere = false;

    if (!wasHere)
    {
        //get name of instance to use
        const char *instanceName;
        GTI_RETURN ret = GtiHelper::getInstanceName (&instanceName);
        assert (ret == GTI_SUCCESS);

        ThreadedAppPlace* place;
        place = ThreadedAppPlace::getInstance(instanceName);
        ThreadedAppPlace::freeInstanceForced (place);

    }
    wasHere = true;
    return 0; //PMPI_Finalize ();
}
#endif

uint64_t   ThreadedAppPlace::numClients=0, ThreadedAppPlace::numChannels=0;


void ThreadedAppPlace::init(){
    //add the id to all sub modules
    char temp[64];
    sprintf (temp, "%" PRIu64, buildLayer());
    addDataToSubmodules ("id", temp);

    //create sub modules
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //save sub modules
    assert (subModInstances.size() >= 2);
    myTraceRecv = (I_CommStrategyDown*) subModInstances[0];

/*    //add the id to all sub modules
    char temp[64];
    GtiTbonNodeInLayerId id;
    getNodeInLayerId(id);
    sprintf (temp,"%lu", id);
    addDataToSubmodules ("id", temp);*/

    myReceival = (I_PlaceReceival*) subModInstances[1];

    //Lets determine the number of  additional modules we have
    //a) Module for intra communication
    int intraModuleIndex = -1; //Index into module array that is a intra comm strategy -1 for no intra comm strategy
    std::map<std::string, std::string> data = getData();
    if (data.find("intra_strat_index") != data.end ())
        intraModuleIndex = atoi(data.find("intra_strat_index")->second.c_str());

    if (intraModuleIndex >= 0)
        myIntraRecv = (I_CommStrategyIntra*) subModInstances[intraModuleIndex];

    //b) Module for wrapper
    int wrapperModuleIndex = 2;
    if (intraModuleIndex == 2)
        wrapperModuleIndex++;

    if (subModInstances.size() > wrapperModuleIndex)
        myWrapper = (I_Module*) subModInstances[wrapperModuleIndex];

    //c) Do we have a flood control?
    bool hasFloodControl = false;
    if (data.find("has_flood_control") != data.end () && atoi(data.find("has_flood_control")->second.c_str()) == 1)
    {
        hasFloodControl = true;
        //myFloodControl = (I_FloodControl*) subModInstances[subModInstances.size()-1];
    }

    //d) Module for profiler (if used)
    if (data.find("has_profiler") != data.end () && atoi(data.find("has_profiler")->second.c_str()) == 1)
    {
        myProfiler = (I_Profiler*) subModInstances[subModInstances.size()-(1+(int)hasFloodControl)];
    }

    //Pair communication strategies with receival
    std::list<I_CommStrategyUp*> upStrats;
    std::list<I_CommStrategyUp*>::iterator upIter;
    myReceival->getUpwardsCommunications(&upStrats);

    if (!upStrats.empty())
        myUpStrats.resize(upStrats.size());

    int i = 0;
    for (upIter = upStrats.begin(); upIter != upStrats.end(); upIter++, i++)
    {
        myUpStrats[i] = *upIter;
    }
}

//=============================
// ThreadedAppPlace
//=============================
ThreadedAppPlace::ThreadedAppPlace (const char* instanceName)
    : ModuleBase<ThreadedAppPlace, I_Place> (instanceName),
      myWrapper (NULL),
      myIntraRecv (NULL),
      myUpStrats (),
      myProfiler (NULL),
      myFloodControl (NULL),
      myIntraCommTime (0),
      myIntraCommCount (0),
      myUpCommTime (0),
      myUpCommCount (0),
      finalize (false)
{
//    registerName(instanceName);
}

//=============================
// ~ThreadedAppPlace
//=============================
ThreadedAppPlace::~ThreadedAppPlace (void)
{
    if (myTraceRecv)
    {
        myTraceRecv->shutdown(GTI_FLUSH, GTI_SYNC);
        destroySubModuleInstance ((I_Module*) myTraceRecv);
        myTraceRecv = NULL;
    }

    if (myReceival)
        destroySubModuleInstance ((I_Module*) myReceival);
    myReceival = NULL;

    if (myWrapper)
        destroySubModuleInstance ((I_Module*) myWrapper);
    myWrapper = NULL;

    if (myIntraRecv)
    {
        myIntraRecv->shutdown(GTI_FLUSH);
        destroySubModuleInstance ((I_Module*) myIntraRecv);
        myIntraRecv = NULL;
    }

    if (myProfiler)
        destroySubModuleInstance ((I_Module*) myProfiler);
    myProfiler = NULL;

    if (myFloodControl)
        destroySubModuleInstance ((I_Module*) myFloodControl);
    myFloodControl = NULL;
}

//=============================
// testBroadcast
//=============================
GTI_RETURN ThreadedAppPlace::testBroadcast (void)
{
		bool hadBroadcastEvent = false;
    if (!receiveAndProcessBroadcastEvent (&hadBroadcastEvent, &finalize))
        return GTI_ERROR; //If it fails silently return - TODO: think of a better solution in the long run

    //Report the test result to the FloodControl
    if (myFloodControl)
    {
        myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_UP, 0);

        if (!hadBroadcastEvent)
            myFloodControl->nextTestDecision();
        else
            myFloodControl->rewindDecision();
    }
    return GTI_SUCCESS;
}

//=============================
// testIntralayer
//=============================
GTI_RETURN ThreadedAppPlace::testIntralayer (void)
{
		bool hadIntraEvent = false;
		if (myIntraRecv) {
        if (!receiveAndProcessIntraLayerEvent (&hadIntraEvent))
            return GTI_ERROR; //If it fails silently return - TODO: think of a better solution in the long run

        //Report the test result to the FloodControl
        if (myFloodControl)
        {
                        myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_INTRA, 0);
            if (!hadIntraEvent)
                myFloodControl->nextTestDecision();
            else
                myFloodControl->rewindDecision();
        }
    }
    return GTI_SUCCESS;
}


//=============================
// run
//=============================
void ThreadedAppPlace::run (void)
{
#ifdef GTI_VT
	VT_TRACER("ThreadedAppPlace::run");
#endif /*GTI_VT*/

	GTI_RETURN (*free_function) (void* free_data, uint64_t num_bytes, void* buf);
	int flag;
	uint64_t   size;
	void            *buf, *free_data;
	uint64_t   numOps = 0;
	GTI_RETURN     ret;
	uint64_t fromChannel;
	SuspensionBufferTree *tree = NULL,
			*node,
			*firstSuspendedNode = NULL,
			*firstNonEmptyQ = NULL;
	uint64_t currTimeout = 0;
	const uint64_t timeoutThreshold = 5000000; //5s
	uint64_t lastContinuousTriggerTime = 0;
	const uint64_t continuousTriggerThreshold = 100000; //.1s
	uint64_t downCommTime = 0, downCommInvocations = 0; //PROFILING: time spent for downwards communication
	bool enforceTimeout = false;

	bool wasIdle = false;
	uint64_t idleStartTime = 0, idleTime = 0;
	uint64_t maxQueueSize = 0;

	GTI_STRATEGY_TYPE nextDirection;
	unsigned int nextChannel=0;

#ifdef GTI_DEBUG
	//Debug
	int gRank;
	MPI_Comm_rank (MPI_COMM_WORLD, &gRank);
	char temp[256];
	sprintf (temp, "place_channel_tree_%d.dot", gRank);
	std::ofstream out (temp);
#endif

        //get number of clients
        bool providesCallback;
        myTraceRecv->registerNewClientCallback(newChannel, providesCallback);
        myTraceRecv->getNumClients(&numChannels);
        numClients=1;
        #ifdef GTI_DEBUG
                printf ("\n\nCLIENTS: %ld\n\n",numClients);
        #endif

        //Init flood control
        uint64_t numIntraPlaces = 0;
        if (myIntraRecv) myIntraRecv->getNumPlaces(&numIntraPlaces);
        if (myFloodControl)
            myFloodControl->init(numClients, myIntraRecv != NULL, numIntraPlaces, myUpStrats.size() > 0);

    //Loop until all channels signaled that they finished
	while (!finalize)
	{
		bool receiveRecord = true; //Used to determine whehter it is necessary to receive a new record or not
		bool hasRecordToProcess = false;
		bool hadIntraEvent = false,
                hadBroadcastEvent = false;
		RecordInfo processRecord;


		// 1) Check whether we have any records in our queues that could be processed
		//     If so select such a record as record to process
		//================================================
		if (tree && !enforceTimeout)
		{
		    uint64_t treeSearchStartTime;
		    if (myProfiler) treeSearchStartTime = getUsecTime();

			if (tree->getQueuedRecord (&processRecord))
			{
#ifdef GTI_VT
			VT_TRACER("ThreadedAppPlace::run::gotQueuedRecord");
#endif /*GTI_VT*/
			    //PROFILING: if we where idle before, store the idle time
			    if (wasIdle)
                {
                    idleTime += treeSearchStartTime - idleStartTime;
                    wasIdle = false;
                }

				//Set that there is a record to process ... (we need not receive something)
				receiveRecord = false;
				hasRecordToProcess = true;

				//Report the FloodControl what we got from the queue
				if (myFloodControl)
				    myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_DOWN, processRecord.recordChannelId->getSubId(processRecord.recordChannelId->getNumUsedSubIds()-1));
			}
		}

		//If available, ask for the most promising channel
		if (myFloodControl && receiveRecord)
		    myFloodControl->getCurrentTestDecision(&nextDirection, &nextChannel);

		// 2) No queued records left, first try intra communication (if available)
		//================================================
		if (receiveRecord && myIntraRecv &&
		    (!myFloodControl || nextDirection == GTI_STRATEGY_INTRA) &&
		    !enforceTimeout)
		{
		    uint64_t intraCommStartTime;
		    if (myProfiler) intraCommStartTime = getUsecTime();

		    if (!receiveAndProcessIntraLayerEvent (&hadIntraEvent))
		        break; //If it fails we abort
		    //IMPORTANT: we do not do a long wait for initialization and we do not influence timeout here!
		    //                    I think this is good, but didn't give this too much thought yet.

		    //Report the test result to the FloodControl
		    if (myFloodControl)
		    {
                        myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_INTRA, 0);
		        if (!hadIntraEvent)
		            myFloodControl->nextTestDecision();
		        else
		            myFloodControl->rewindDecision();
		    }

		    //PROFILING: add to idle time if we where idle before
		    if (hadIntraEvent && wasIdle)
		    {
		            idleTime += intraCommStartTime - idleStartTime;
		            wasIdle = false;
		    }
		}

		// 3) No queued records left, also try incoming broadcasts
		//================================================
		if (receiveRecord &&
		    (!myFloodControl || nextDirection == GTI_STRATEGY_UP) &&
		    !enforceTimeout)
		{
		    uint64_t upCommStartTime;
		    if (myProfiler) upCommStartTime = getUsecTime();

		    if (!receiveAndProcessBroadcastEvent (&hadBroadcastEvent, &finalize))
		        break; //If it fails we abort
		    //IMPORTANT: we do not do a long wait for initialization and we do not influence timeout here!
		    //                    I think this is good, but didn't give this too much thought yet.

		    //Report the test result to the FloodControl
		    if (myFloodControl)
		    {
		        myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_UP, 0);

		        if (!hadBroadcastEvent)
		            myFloodControl->nextTestDecision();
		        else
		            myFloodControl->rewindDecision();
		    }

		    //PROFILING: add to idle time if we where idle before
		    if (hadBroadcastEvent && wasIdle)
		    {
		        idleTime += upCommStartTime - idleStartTime;
		        wasIdle = false;
		    }

		    if (finalize)
		    {
		        /**
		         * If this is the finalize notify, lets finish intra communication a second time!
		         */
		        if (!finishIntraCommunication ())
		            break;
		    }
		}

		// 3) No queued records left, receive a new record (inter layer communication)
		//================================================
		if ( enforceTimeout ||
		     (receiveRecord && (!myFloodControl || nextDirection == GTI_STRATEGY_DOWN)))
		{
#ifdef GTI_VT
			{
			VT_TRACER("ThreadedAppPlace::run::test");
#endif /*GTI_VT*/

			uint64_t usecDownCommStart;
			if (myProfiler) usecDownCommStart = getUsecTime();
			flag = 0;

			if (!enforceTimeout)
			    ret = myTraceRecv->test(&flag, &size, &buf, &free_data, &free_function, &fromChannel, nextChannel);
			//ret = myTraceRecv->wait(&size, &buf, &free_data, &free_function, &fromChannel);
#ifdef GTI_VT
			}
#endif /*GTI_VT*/

			if (ret == GTI_ERROR)
			{
				std::cerr << "Place: error while receiving, communication strategy returned error." << std::endl;
				break;
			}

			if (ret == GTI_SUCCESS) //Necessary as strategy may also return it is not ready yet ...
			{
			    //Report the test result to the FloodControl
			    if (myFloodControl && !enforceTimeout)
			    {
			        if (!flag)
			        {
			            myFloodControl->nextTestDecision();
			            myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_DOWN, nextChannel);
			        }
			        else
			        {
			            myFloodControl->rewindDecision();
			            myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_DOWN, fromChannel);
			        }
			    }

			    if (flag)
			    {
			        //PROFILING: if successful communication, store the amount of time spent for communication!
			        //                  also if we where idle, add the idle time (that is the time till the communication started)
			        if (myProfiler)
			        {
			            downCommTime += getUsecTime() - usecDownCommStart;
			            downCommInvocations++;
			        }

			        if (wasIdle)
			        {
			        	idleTime += usecDownCommStart - idleStartTime;
			        	wasIdle = false;
			        }

#ifdef GTI_VT
			        VT_TRACER("ThreadedAppPlace::run::evaluateNewRecord");
#endif /*GTI_VT*/
			        I_ChannelId *channelId;
			        bool isFinalizer = false;
			        bool isOutOfOrder = false;

			        //Get and update channel id for this record
			        if (myReceival->getUpdatedChannelId(buf, size, fromChannel, numChannels, &channelId, &isFinalizer, &isOutOfOrder) != GTI_SUCCESS)
			            break; //Abort main loop in case of some error

			        //Is it the last finalizer?
			        if (isFinalizer)
			        {
			            //We need to fininsh up intra layer communication before we continue
			            /*
			             * We now finish intra communication when the real GTI internal finalize event arrives!
			             */
			            if (!finishIntraCommunication ())
			                break;
			        }

			        //Store information on the record
			        processRecord.buf = buf;
			        processRecord.free_data = free_data;
			        processRecord.num_bytes = size;
			        processRecord.recordChannelId = channelId;
			        processRecord.buf_free_function = free_function;

			        //Allocate tree if this is the first channel id we got
			        if (!tree) tree = new SuspensionBufferTree (channelId->getNumUsedSubIds()-1, 1);

			        //==If we have any suspension, we must check whether it is allowed to process the record
			        if (!tree->hasAnySuspension() || isOutOfOrder)
			        {
			            hasRecordToProcess = true;
			        }
			        else
			        {
			            //Find the node for this channel id in the channel id tree
			            firstSuspendedNode = firstNonEmptyQ = NULL;
			            node = tree->getNode(channelId, &firstSuspendedNode, &firstNonEmptyQ);

			            //Is the channel of the received record ok for processing ?
			            //If there is any non-empty queue, then no
			            if (firstNonEmptyQ)
			            {
			                //If there was a non empty queue on the way, add to that node, otherwise add to the leaf node
			                firstNonEmptyQ->pushBack(processRecord);

			                if (tree->getChildQueueSize() + tree->getQueueSize() > maxQueueSize)
			                    maxQueueSize = tree->getChildQueueSize() + tree->getQueueSize();

			                //Did queing here just create a completely suspended tree?
			                if (tree->isCompletlySuspended())
			                {
			                    enforceTimeout = true;
			                }

			                //FloodControl: mark as bad record
			                if (myFloodControl)
			                    myFloodControl->markCurrentRecordBad();

#ifdef GTI_DEBUG
			                ////Debug:
			                //std::cout << "Test: " << processRecord.recordChannelId->toString() << std::endl;
			                out << "digraph temp" << std::endl << "{" << std::endl << "myNode [label=\"Added to queue from: "<< channelId->toString() << "\"];" << std::endl << "}" << std::endl;
			                tree->printAsDot(out);
#endif
			            }
			            else
			                if (firstSuspendedNode)
			                {
			                    //Can't process at the moment, has suspended sub-nodes or parents!
			                    node->pushBack(processRecord);

			                    if (tree->getChildQueueSize() + tree->getQueueSize() > maxQueueSize)
			                        maxQueueSize = tree->getChildQueueSize() + tree->getQueueSize();

			                    //Did queing here just create a completely suspended tree?
			                    if (tree->isCompletlySuspended())
			                    {
			                        enforceTimeout = true;
			                    }

			                    //FloodControl: mark as bad record
			                    if (myFloodControl)
			                        myFloodControl->markCurrentRecordBad();

#ifdef GTI_DEBUG
			                    ////Debug:
			                    //std::cout << "Test: " << processRecord.recordChannelId->toString() << std::endl;
			                    out << "digraph temp" << std::endl << "{" << std::endl << "myNode [label=\"Added to queue from(descendant blocked): "<< channelId->toString() << "\"];" << std::endl << "}" << std::endl;
			                    tree->printAsDot(out);
#endif
			                }
			                else
			                    if (node->getChildQueueSize())
			                    {
			                        //Can't process at the moment, there is a queued event in the childs that needs to be processed first
			                        node->pushBack(processRecord);

			                        if (tree->getChildQueueSize() + tree->getQueueSize() > maxQueueSize)
			                            maxQueueSize = tree->getChildQueueSize() + tree->getQueueSize();

			                        //Did queing here just create a completely suspended tree?
			                        if (tree->isCompletlySuspended())
			                        {
			                            enforceTimeout = true;
			                        }

			                        //FloodControl: mark as bad record
			                        if (myFloodControl)
			                            myFloodControl->markCurrentRecordBad();

#ifdef GTI_DEBUG
			                        ////Debug:
			                        //std::cout << "Test: " << processRecord.recordChannelId->toString() << std::endl;
			                        out << "digraph temp" << std::endl << "{" << std::endl << "myNode [label=\"Added to queue from(descendant has queue entry): "<< channelId->toString() << "\"];" << std::endl << "}" << std::endl;
			                        tree->printAsDot(out);
#endif
			                    }
			                    else
			                    {
			                        hasRecordToProcess = true;
			                    }
			        }//If (!tree->hasAnySuspension())
			    }//test had flag=true
			    else //flag
			    {
			        //we did not receive something ...
			        if (!hadIntraEvent && !hadBroadcastEvent)
			        {
#ifdef GTI_VT
			            VT_TRACER("ThreadedAppPlace::run::sleep");
#endif /*GTI_VT*/

			            if (!wasIdle)
			            {
			                idleStartTime = getUsecTime ();
			                wasIdle = true;
			            }

			            //Timeout threshold reached ?
			            if (getUsecTime() - idleStartTime > timeoutThreshold ||
			                enforceTimeout)
			            {
			                enforceTimeout = false;

			                //PROFILING: if we where idle, stop idle time now, the upcoming is split into infrastructure and timeout time
			                if (wasIdle)
			                {
		                        idleTime += getUsecTime() - idleStartTime;
			                    wasIdle = false;
			                }

			                currTimeout = 0;

			                myReceival->timeOutReductions();
			                if (tree)
			                {
			                    //Send missing acknowledge signals!
			                    std::list<long> list = tree->getChildsIndicesWithSuspension ();
			                    std::list<long>::iterator chanIter;
			                    for (chanIter = list.begin(); chanIter != list.end(); chanIter++)
			                    {
			                        /*See note in the other acknowledges*/
  		                            myTraceRecv->acknowledge (*chanIter);
			                    }

			                    //Clear suspensions
			                    tree->removeAllSuspensions();
			                }

#ifdef GTI_DEBUG
			                //DEBUG:
			                std::cout << "Timed out!" << std::endl;
			                out << "digraph temp" << std::endl << "{" << std::endl << "myNode [label=\"Time out\"];" << std::endl << "}" << std::endl;
			                if (tree) tree->printAsDot(out);
#endif
			                //							}
			            }
			        }//We did not had an inter event and no intra event too -> sleep
			    }//if flag
			}//if GTI_SUCCESS
			else
			{
			    if (!wasIdle)
			    {
			        idleStartTime = getUsecTime ();
			        wasIdle = true;
			    }

			    //Input strategy not ready yet, wait a bit ...
			    usleep(500);
			}//if-else-if for return of wait call
		}//If need to receive record

		//4) Process a record if there was a valid record for processing
		//================================================
		if (hasRecordToProcess)
		{
#ifdef GTI_VT
			VT_TRACER("ThreadedAppPlace::run::process");
#endif /*GTI_VT*/
			//reset timeout
			currTimeout = 0;

			//process
			bool wasChannelClosed = false;
			std::list<I_ChannelId*> reopenedChannels;
			std::list<I_ChannelId*>::iterator iter;
			int fromChannel = processRecord.recordChannelId->getSubId(processRecord.recordChannelId->getNumUsedSubIds()-1);

			if (myReceival->ReceiveRecord(
					processRecord.buf,
					processRecord.num_bytes,
					processRecord.free_data,
					processRecord.buf_free_function,
					&numClients,
					processRecord.recordChannelId,
					&wasChannelClosed,
					&reopenedChannels) != GTI_SUCCESS)
				break; //Abort main loop in case of some error

			//evaluate suspensions
			if (wasChannelClosed)
			{
				node = tree->getNode(processRecord.recordChannelId, NULL, NULL);
				node->setSuspension(true, processRecord.recordChannelId);

				if (tree->isCompletlySuspended())
				{
				    enforceTimeout = true;

				    /**
				     * DEBUG
				     */
				    /*
				    std::ofstream out;
				    std::stringstream name;
				    static int x = 0;
				    name << "force_timeout_tree_" << getpid() << "_" << x << ".dot";
				    x++;
				    out.open(name.str().c_str());
				    tree->printAsDot(out);
				    out.close();
				     */
				}
			}
			else
			{
			    //Done, acknowledge now!
			    /*
			     *Note: that we only achnowledge if an event is not involved in a reduction
			     *         or its reduction succeeded has concsequences. With this layout we
			     *         can only perform a reduction if we only need one event from each
			     *         channel. I.e. we are able to reduce some event on each level!
			     *         Usually that shouldn't be a problem, but it can cause unexpected behavior.
			     */
			    myTraceRecv->acknowledge (fromChannel);
			}

			for (iter = reopenedChannels.begin(); iter != reopenedChannels.end(); iter++)
			{
				I_ChannelId *id = *iter;
				if (!id) continue;

				node = tree->getNode(id, NULL, NULL);
				node->setSuspension(false, id);

				//Send an acknowledge of a finally processed event
				/*See note in the other acknowledge*/
				myTraceRecv->acknowledge (id->getSubId(id->getNumUsedSubIds()-1));

				delete (id);
			}

			/*
			 * We are almost done, the last ancestor tool place told us he processed the shutdown event
			 */
			if (numClients <= 0)
			{
			    /*
			     * If this is a top layer (no descendants) we have to set finalize ourselves,
			     * as we won't receive a finalize event in that case
			     */
			    if (myUpStrats.empty())
			        finalize = true;

			    /*
			     * We are done receiving from downwards, lets flush our outgoing strategies!
			     */
			    for (int s = 0; s < myUpStrats.size(); s++)
			    {
			        myUpStrats[s]->flush();
			    }
			}

#ifdef GTI_DEBUG
			////Debug:
			//std::cout << "Test: " << processRecord.recordChannelId->toString() << std::endl;
			out << "digraph temp" << std::endl << "{" << std::endl << "myNode [label=\"Processed from: "<< processRecord.recordChannelId->toString() << "\"];" << std::endl << "}" << std::endl;
			tree->printAsDot(out);
#endif

			delete (processRecord.recordChannelId);
			numOps++;
		}//if has record to process

		//5) Check whether we should trigger continuous analyses
		//================================================
		uint64_t usecCurr = getUsecTime();
		if (usecCurr - lastContinuousTriggerTime > continuousTriggerThreshold)
		{
			uint64_t timeSinceLast = usecCurr - lastContinuousTriggerTime;
			if (lastContinuousTriggerTime == 0)
				timeSinceLast = 0;
			lastContinuousTriggerTime = usecCurr;

			myReceival->triggerContinuous(timeSinceLast);
		}
	}//while channels left

#ifdef GTI_DEBUG
	//Debug
	std::cout << "NumOps: " << numOps << std::endl;
#endif

	//Delete tree
	if (tree) delete (tree);

	//PROFILING: report
	if (myProfiler)
	{
	    myProfiler->reportIdleTime(idleTime);
	    myProfiler->reportDownCommTime(downCommTime, downCommInvocations);
	    myProfiler->reportUpCommTime(myUpCommTime, myUpCommCount);
	    myProfiler->reportIntraCommTime(myIntraCommTime, myIntraCommCount);
	    if (myFloodControl)
	        myProfiler->reportMaxBadness(myFloodControl->getMaxBadness());

	    myProfiler->reportWrapperAnalysisTime ("ThreadedMpiPlace", "maxEventQueue", 0, maxQueueSize);
	}
//    freeInstanceForced(this);
}//run method

//=============================
// receiveAndProcessIntraLayerEvent
//=============================
bool ThreadedAppPlace::receiveAndProcessIntraLayerEvent (bool *hadEvent)
{
    GTI_RETURN (*free_function) (void* free_data, uint64_t num_bytes, void* buf);
    int flag;
    uint64_t   size;
    void            *buf, *free_data;
    GTI_RETURN     ret;
    uint64_t fromChannel;

    if (hadEvent)
        *hadEvent = false;

    if (!myIntraRecv)
        return true;

    uint64_t intraCommStartTime;
    if (myProfiler) intraCommStartTime = getUsecTime ();

    ret = myIntraRecv->test(&flag, &fromChannel, &size, &buf, &free_data, &free_function);

    if (ret == GTI_ERROR)
    {
        std::cerr << "Place: error while receiving, communication strategy returned error." << std::endl;
        return false;
    }

    if (ret == GTI_SUCCESS && flag)
    {
        //Set information for flood control
        if (myFloodControl)
            myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_INTRA, fromChannel);

        if (myProfiler)
        {
            myIntraCommTime = getUsecTime () - intraCommStartTime;
            myIntraCommCount++;
        }

        if (myReceival->ReceiveIntraRecord(
                buf,
                size,
                free_data,
                free_function,
                fromChannel) != GTI_SUCCESS)
            return false; //Abort main loop in case of some error

        if (hadEvent)
            *hadEvent = true;
    }

    return true;
}

//=============================
// receiveAndProcessBroadcastEvent
//=============================
bool ThreadedAppPlace::receiveAndProcessBroadcastEvent (bool *hadEvent, bool *pOutFinalize)
{
    GTI_RETURN (*free_function) (void* free_data, uint64_t num_bytes, void* buf);
    int flag = false;
    uint64_t   size;
    void            *buf, *free_data;
    GTI_RETURN     ret;
    bool finalize = false;

    if (hadEvent)
        *hadEvent = false;

    if (pOutFinalize)
        *pOutFinalize = false;

    for (int i = 0; i < myUpStrats.size() && !flag; i++)
    {
        uint64_t upCommStartTime;
        if (myProfiler) upCommStartTime = getUsecTime ();

        ret = myUpStrats[i]->test(&flag, &size, &buf, &free_data, &free_function);

        if (ret == GTI_ERROR)
        {
            std::cerr << "Place: error while receiving from upwards, communication strategy returned error." << std::endl;
            return false;
        }

        if (ret == GTI_SUCCESS && flag)
        {
            //Set information for flood control
            if (myFloodControl)
                myFloodControl->setCurrentRecordInfo(GTI_STRATEGY_UP, 0); //We ignore the existence of multiple upwards strategies here

            if (myProfiler)
            {
                myUpCommTime = getUsecTime () - upCommStartTime;
                myUpCommCount++;
            }

            if (myReceival->ReceiveBroadcastRecord(
                    buf,
                    size,
                    free_data,
                    free_function,
                    &finalize) != GTI_SUCCESS)
                return false; //Abort main loop in case of some error

            if (hadEvent)
                *hadEvent = true;
        }
    }

    if (pOutFinalize)
        *pOutFinalize = finalize;

    return true;
}

//=============================
// finishIntraCommunication
//=============================
bool ThreadedAppPlace::finishIntraCommunication (void)
{
    if (!myIntraRecv)
        return true;

    bool hadEvent = false;
    bool isFinished = false;

    //Loop till intra communication is finished
    do
    {
        //Receive as much as we can right now
        do
        {
            if (!receiveAndProcessIntraLayerEvent (&hadEvent))
                return false;
        } while (hadEvent);

        //Check whether we are done
        myIntraRecv->communicationFinished(&isFinished);

        //Receive anything we might have got during the finish
        do
        {
            if (!receiveAndProcessIntraLayerEvent (&hadEvent))
                return false;
        } while (hadEvent);
    }
    while (!isFinished);

    return true;
}

//=============================
// getNodeInLayerId
//=============================
GTI_RETURN ThreadedAppPlace::getNodeInLayerId (GtiTbonNodeInLayerId* id)
{
  return myTraceRecv->getPlaceId(id);
}

//=============================
// getLayerIdForApplicationRank
//=============================
GTI_RETURN ThreadedAppPlace::getLayerIdForApplicationRank (int rank, GtiTbonNodeInLayerId* id)
{
  return GTI_SUCCESS;
}


/*EOF*/
