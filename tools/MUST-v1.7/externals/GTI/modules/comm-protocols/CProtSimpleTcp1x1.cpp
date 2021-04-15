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
 * @file CProtSimpleTcp1x1.cpp
 *       A TCP based protocol that was derived from CProtSimpleTcp, but that avoids the use of the
 *       awesome double ping-pong. Rather this should be a reasonably performing implementation.
 *       IMPORTANT: the implementation is fixed to a 1-to-1 connection! More requires an extension.
 *
 * @author Tobias Hilbrich
 * @date 17.11.2014
 */

#include <assert.h>
#include <stdio.h>
#include <pnmpimod.h>
#include "CProtSimpleTcp.h"
#include "GtiMacros.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

using namespace gti;

mGET_INSTANCE_FUNCTION(CProtSimpleTCP)
mFREE_INSTANCE_FUNCTION(CProtSimpleTCP)
mPNMPI_REGISTRATIONPOINT_FUNCTION(CProtSimpleTCP)

const uint64_t CProtSimpleTCP::TOKEN_MSG_SIZE_HEADER = 0xFFFFFFFF;
const uint64_t CProtSimpleTCP::TOKEN_SYNC_MSG        = 0xFFFFFFFE;

//=============================
// Function to perform a blocking receive with sockets.
//=============================
int blocking_recv(int socket, void* buffer, size_t bytes)
{
    int retBytes;
    do
    {
        retBytes = ::recv (socket,buffer,bytes, MSG_WAITALL);

        if (retBytes < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                usleep(1); //TODO specifiy something more meaningful
                retBytes = 0;
                continue;
            }
        }

        if (retBytes < 0)
        {
            int myErrno = errno;
            printf ("blocking_recv failed with errno=%d\n",myErrno);
            printf ("          Error: %s\n",strerror(myErrno));
            assert (0);
        }
        else
        {
            usleep(1); //TODO specifiy something more meaningful
        }
    }
    while (retBytes == 0);

    return retBytes;
}

//=============================
// Function to perform a blocking send with sockets.
//=============================
int all_send(int socket, void* buffer, size_t bytes)
{
    int sentBytes;
    do
    {
        sentBytes = ::send (socket,buffer,bytes, 0);

        if (sentBytes < 0)
        {
            int myErrno = errno;
            printf ("CRITICAL: all_send failed with errno=%d\n",myErrno);
            printf ("          Error: %s\n",strerror(myErrno));
            assert (0);
        }
        else
        {
            usleep(1); //TODO specifiy something more meaningful
        }
    }
    while (sentBytes == 0);

    return sentBytes;
}

//=============================
// CProtSimpleTCP
//=============================
CProtSimpleTCP::CProtSimpleTCP (const char* instanceName)
: ModuleBase<CProtSimpleTCP, I_CommProtocol> (instanceName),
  myIsShutdown (false),
  myPlaceId (0),
  myNextRequestID (1)
{
    std::vector<I_Module*> subModInstances;
    subModInstances = createSubModuleInstances ();

    //A Comm Protocol likely needs no sub modules
    assert (subModInstances.empty());

    //Init data
    myNumSockets = 0;
    mySockets = NULL;
    myOutstandingMsgs.clear();
    myActiveRequests.clear();
    myCompletedRequests.clear();

    //Startup if necessary
    GTI_RETURN ret = startup ();
    assert (ret == GTI_SUCCESS);
}

//=============================
// ~CProtSimpleTCP
//=============================
CProtSimpleTCP::~CProtSimpleTCP (void)
{
    //Closing of sockets is done in shutdown !

    if (mySockets)
        delete mySockets;
    mySockets = NULL;
}

//=============================
// isConnected
//=============================
bool CProtSimpleTCP::isConnected (void)
{
    return isInitialized() && !isFinalized();
}

//=============================
// isInitialized
//=============================
bool CProtSimpleTCP::isInitialized (void)
{
    /*Constructor either succeeds in connecting or dies with an assert*/
    return true;
}

//=============================
// isFinalized
//=============================
bool CProtSimpleTCP::isFinalized (void)
{
    return myIsShutdown;
}

//=============================
// startup
//=============================
GTI_RETURN CProtSimpleTCP::startup (void)
{
    unsigned int        commId;
    char                commSide;
    uint64_t       tierSize;
    uint64_t       selfID;
    unsigned short int  serverPort;
    struct sockaddr_in  serverAddr;
    int                 serverSocket;
    static bool isConnected = false;

    if (isConnected)
            return GTI_SUCCESS;
    isConnected = true;
    int ret;

    assert (!myIsShutdown);

    // === (1) Read data ===
    //General protocol data
    std::map<std::string, std::string> data = getData ();
    std::map<std::string, std::string>::iterator i;

    i=data.find("comm_id");
    assert (i != data.end ());
    commId = atoi (i->second.c_str());

    i=data.find("side");
    assert (i != data.end ());
    assert (i->second.length() == 1); //just one letter !
    commSide = i->second.c_str()[0];
    assert ((commSide == 't') || (commSide == 'b')); //either "t" or "b"

    i=data.find("tier_size");
    assert (i != data.end ());
    tierSize = atol (i->second.c_str());

    i=data.find("id");
    assert (i != data.end ());
    selfID = atol (i->second.c_str());
    myPlaceId  = selfID;

    //TCP specific stuff
    i=data.find("server_port");
    if (i != data.end ())
    {
        serverPort = atoi (i->second.c_str());
    }
    else
    {
        if (getenv("GTI_TCP_SERVER_PORT") != NULL)
        {
            serverPort = atoi (getenv("GTI_TCP_SERVER_PORT"));
        }
        else
        {
            std::cerr << __FILE__ << ":" << __LINE__ << " => Could not determine TCP server port, either have \"server_port\" in the module data or specify the environmental \"GTI_TCP_SERVER_PORT\"!" << std::endl;
            assert (0);
        }
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    i=data.find("server_ip");
    if (i != data.end ())
    {
        inet_aton(i->second.c_str(), &(serverAddr.sin_addr));
    }
    else
    {
        if (getenv("GTI_TCP_SERVER_IP") != NULL)
        {
            inet_aton(getenv("GTI_TCP_SERVER_IP"), &(serverAddr.sin_addr));
        }
        else
        {
            std::cerr << __FILE__ << ":" << __LINE__ << " => Could not determine TCP server IP address, either have \"server_ip\" in the module data or specify the environmental \"GTI_TCP_SERVER_IP\"!" << std::endl;
            assert (0);
        }
    }
    serverAddr.sin_port = serverPort;
    serverAddr.sin_family = AF_INET;

    // === (2) Connect to server ===
    serverSocket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert (serverSocket >= 0);

    ret = ::connect(serverSocket,
                    (struct sockaddr *) &serverAddr,
                    sizeof(serverAddr)
                   );
    if (ret < 0)
    {
        int myErrno = errno;
        printf ("connect failed with errno=%d\n",myErrno);
        printf ("          Error: %s\n",strerror(myErrno));
    }
    assert (ret >= 0);

    // === (3) Send own connection info to server ===
    // We send:
    //   commId, commSide, tierSize, selfID, [OTF_ProcID]
    uint64_t connectionInfoBuffer[4];
    unsigned int len;

    connectionInfoBuffer[0] = (uint64_t) commId;
    if (commSide == 't')
        connectionInfoBuffer[1] = (uint64_t) 0;
    else
        connectionInfoBuffer[1] = (uint64_t) 1;
    connectionInfoBuffer[2] = (uint64_t) tierSize;
    connectionInfoBuffer[3] = (uint64_t) selfID;

    len = sizeof(uint64_t) * 4;

    ret = ::all_send(serverSocket, connectionInfoBuffer, len);
    assert (ret == len);

    // === (4) Receive partner bundle from server ===
    int bytes = 0;

    //Receive num bundles
    bytes = ::blocking_recv(serverSocket, connectionInfoBuffer, sizeof(uint64_t));
    assert (bytes == sizeof (uint64_t));

    //Receive bundle contents
    len = sizeof (uint64_t) * connectionInfoBuffer[0] * 2;
    uint64_t *connectionPartners = new uint64_t[connectionInfoBuffer[0]*2];

    bytes = ::blocking_recv(serverSocket, connectionPartners, len);
    assert (bytes == len);

    // === (5) Open connection to partners ===
    myNumSockets = connectionInfoBuffer[0];
    myOutstandingMsgs.resize (myNumSockets);
    myActiveRequests.resize (myNumSockets+1);

    assert (!mySockets);
    mySockets = new int[myNumSockets];

    for (unsigned int i = 0; i < myNumSockets; i++)
    {
        struct sockaddr_in  partnerAddr;
        memset(&partnerAddr, 0, sizeof(partnerAddr));
        partnerAddr.sin_addr.s_addr = connectionPartners[2*i + 0];
        partnerAddr.sin_port = connectionPartners[2*i + 1];
        partnerAddr.sin_family = AF_INET;

        printf("Establishing connection: ip=%s port=%d\n",inet_ntoa(partnerAddr.sin_addr),partnerAddr.sin_port);

        mySockets[i] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        assert (mySockets[i] >= 0);

        if (commSide == 'b')
        {
            int listenSocket = mySockets[i];

            //only one sock on bottom side
            assert (myNumSockets == 1);

            //Lets patch the addr to not use a remote IP!
            partnerAddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */

            //Bottom side listens
            ret = bind(
                    listenSocket,
                    (struct sockaddr*) &partnerAddr,
                    sizeof(partnerAddr)
                    );
            if (ret < 0)
            {
                int myErrno = errno;
                printf ("During connecting up: bind (socket=%d) failed with errno=%d\n", listenSocket, myErrno);
                printf ("          Error: %s\n",strerror(myErrno));
            }
            assert(ret  >= 0);

            ret = listen(listenSocket, 1);
            assert (ret >= 0);

            //Send "I-am-listing" to the server
            bytes = ::all_send(serverSocket, connectionInfoBuffer, sizeof(uint64_t));
            assert (bytes == sizeof(uint64_t));

            unsigned int addrLen = sizeof(partnerAddr);
            mySockets[i] = accept(listenSocket, (struct sockaddr *) &partnerAddr, &addrLen);
            assert (mySockets[i] >= 0);

            close (listenSocket);
        }
        else
        {
            //wait for a "good-to-connect" from the server
            bytes = ::blocking_recv(serverSocket, connectionInfoBuffer, sizeof(uint64_t));
            assert (bytes == sizeof(uint64_t));

            //Top side connects
            ret = connect(mySockets[i],
                    (struct sockaddr *) &partnerAddr,
                    sizeof(partnerAddr)
                   );
            assert (ret >= 0);
        }
    }
    delete[] (connectionPartners);

    // === (6) Send a done connecting to the server ===
    len = sizeof (uint64_t);
    ret = all_send(serverSocket, connectionInfoBuffer, len);
    assert (ret == len);

    // === (7) Close connection to server ===
    close (serverSocket);

    return GTI_SUCCESS;
}

//=============================
// shutdown
//=============================
GTI_RETURN CProtSimpleTCP::shutdown (void)
{
    for (unsigned int i = 0; i < myNumSockets; i++)
    {
        if (mySockets[i] >= 0)
            close (mySockets[i]);
        mySockets[i] = -1;
    }

    myIsShutdown = true;

    return GTI_SUCCESS;
}

//=============================
// removeOutstandingRequests
//=============================
GTI_RETURN CProtSimpleTCP::removeOutstandingRequests (void)
{
    //num sockets +1 for any receives ...
    for (uint64_t i = 0; i <= myNumSockets; i++)
    {
        myActiveRequests[i].clear ();
    }

    return GTI_SUCCESS;
}

//=============================
// getPlaceId
//=============================
GTI_RETURN CProtSimpleTCP::getPlaceId (uint64_t* outPlaceId)
{
    if (outPlaceId)
        *outPlaceId = myPlaceId;

    return GTI_SUCCESS;
}

//=============================
// getNumChannels
//=============================
GTI_RETURN CProtSimpleTCP::getNumChannels (
        uint64_t* out_numChannels)
{
    if (out_numChannels)
        *out_numChannels = myNumSockets;
    return GTI_SUCCESS;
}

//=============================
// ssend
//=============================
GTI_RETURN CProtSimpleTCP::ssend (
        void* buf,
        uint64_t num_bytes,
        uint64_t channel
        )
{
    ssize_t sentBytes;

    assert ( channel < myNumSockets );

    //Send the actual message
    sentBytes = ::all_send (mySockets[channel], buf, num_bytes);
    assert (sentBytes == num_bytes);

    return GTI_SUCCESS;
}

//=============================
// isend
//=============================
GTI_RETURN CProtSimpleTCP::isend (
        void* buf,
        uint64_t num_bytes,
        unsigned int *out_request,
        uint64_t channel
        )
{
    /**
     * @todo this is a mock up implementation that does not really supports isend
     */
    ssend (buf, num_bytes, channel);

    if (out_request)
        *out_request = myNextRequestID;

    myCompletedRequests.insert(
            std::make_pair(
                    myNextRequestID,
                    std::make_pair (channel, num_bytes)));

    myNextRequestID++;

    return GTI_SUCCESS;
}

//=============================
// recv
//=============================
GTI_RETURN CProtSimpleTCP::recv (
        void* out_buf,
        uint64_t num_bytes,
        uint64_t* out_length,
        uint64_t channel,
        uint64_t *out_channel
        )
{
    ssize_t sentBytes;

    assert ( ( channel < myNumSockets ) ||
             (channel == RECV_ANY_CHANNEL)                  );

    /**
     * @todo This implementation is fixed to 1-to-1 socket connections!
     *
     * @todo IMPORTANT: This violates order if we mix recv's with irecv's (hopefully we don't in our use case)
     */
    sentBytes = ::blocking_recv (mySockets[0], out_buf, num_bytes);
    assert (sentBytes == num_bytes);

    if (out_length)
        *out_length = num_bytes;
    if (out_channel)
        *out_channel = 0;

    return GTI_SUCCESS;
}

//=============================
// irecv
//=============================
GTI_RETURN CProtSimpleTCP::irecv (
        void* out_buf,
        uint64_t num_bytes,
        unsigned int *out_request,
        uint64_t channel
        )
{
    static uint64_t order = 0;

    /**
     * @todo This implementation is fixed to 1-to-1 socket connections!
     */

    myActiveRequests[0].push_back (
            std::make_pair (
                    myNextRequestID,
                    std::make_pair (
                            order,
                            std::make_pair (out_buf,num_bytes)
                            )
                    )
            );
    if (out_request)
        *out_request = myNextRequestID;
    myNextRequestID++;
    order++;
    return GTI_SUCCESS;
}

//=============================
// test_msg
//=============================
GTI_RETURN CProtSimpleTCP::test_msg (
        unsigned int request,
        int* out_completed,
        uint64_t* out_receive_length,
        uint64_t* out_channel
        )
{
    //not completed ...
    if (out_completed)
        *out_completed = 0;

    /**
     * @todo This implementation is fixed to 1-to-1 socket connections!
     */

    //(1) See whether request already in completed requests
    std::map <unsigned int, CompletionInfo>::iterator iter = myCompletedRequests.find(request);
    if (iter != myCompletedRequests.end())
    {
        if (out_receive_length)
            *out_receive_length = iter->second.second;
        if (out_channel)
            *out_channel = iter->second.first;
        if (out_completed)
            *out_completed = 1;
        myCompletedRequests.erase (iter);
        return GTI_SUCCESS;
    }

    //(2) Try the first request we have
    assert (!myActiveRequests[0].empty());

    Request r = myActiveRequests[0].front();
    RequestInfo info = r.second;
    unsigned int requestId = r.first;

    uint64_t order = info.first;
    void* buf = info.second.first;
    uint64_t len = info.second.second;

    int retBytes = ::recv (mySockets[0], buf, 4,  MSG_PEEK | MSG_DONTWAIT); /**< Use a length of 4 bytes just to peek.*/

    if (retBytes > 0)
    {
        //Bingo we have something to receive
        int rcvBytes = ::blocking_recv (mySockets[0], buf, len);
        assert (rcvBytes <= len);

        //Remove from active requests
        myActiveRequests[0].pop_front();

        //Was this the asked for request?
        if (requestId == request)
        {
            if (out_completed)
                *out_completed = 1;
            if (out_receive_length)
                *out_receive_length = rcvBytes;
            if (out_channel)
                *out_channel = 0;
        }
        else
        {
            myCompletedRequests.insert (
                    std::make_pair (requestId,
                            std::make_pair(0, rcvBytes)
                    ));
        }
    }
    else
    {
        if (errno != EAGAIN &&  errno != EINTR && errno != EWOULDBLOCK)
        {
            int myErrno = errno;
            printf ("recv (peek) failed with errno=%d\n",myErrno);
            printf ("          Error: %s\n",strerror(myErrno));
            assert (0);
        }
    }

    return GTI_SUCCESS;
}

//=============================
// wait_msg
//=============================
GTI_RETURN CProtSimpleTCP::wait_msg (
        unsigned int request,
        uint64_t* out_receive_length,
        uint64_t* out_channel
        )
{
    /**
     * @todo This implementation is fixed to 1-to-1 socket connections!
     */

    //(1) See whether request already in completed requests
    std::map <unsigned int, CompletionInfo>::iterator iter = myCompletedRequests.find(request);
    if (iter != myCompletedRequests.end())
    {
        if (out_receive_length)
            *out_receive_length = iter->second.second;
        if (out_channel)
            *out_channel = iter->second.first;
        myCompletedRequests.erase (iter);
        return GTI_SUCCESS;
    }

    //(2) Try the first request we have
    bool completed = false;
    while (!completed)
    {
        //There must be a request in the queue if we aren't completed yet
        assert (!myActiveRequests[0].empty());

        Request r = myActiveRequests[0].front();
        RequestInfo info = r.second;
        unsigned int requestId = r.first;

        uint64_t order = info.first;
        void* buf = info.second.first;
        uint64_t len = info.second.second;

        //Bingo we have something to receive
        int rcvBytes = ::blocking_recv (mySockets[0], buf, len);
        assert (rcvBytes <= len);

        //Remove from active requests
        myActiveRequests[0].pop_front();

        //Was this the asked for request?
        if (requestId == request)
        {
            completed = true;

            if (out_receive_length)
                *out_receive_length = rcvBytes;
            if (out_channel)
                *out_channel = 0;
        }
        else
        {
            myCompletedRequests.insert (
                    std::make_pair (requestId,
                            std::make_pair(0, rcvBytes)
                    ));
        }
    }//while (!completed)

    return GTI_SUCCESS;
}

//=============================
// recv_msg_content
//=============================
GTI_RETURN CProtSimpleTCP::recv_msg_content (uint64_t channel, uint64_t size, void* target_buffer, bool *target_was_used, bool *request_was_used, unsigned int *request_finished)
{
    ssize_t sentBytes;
    uint64_t header[2];
    ssize_t headerLen = sizeof (uint64_t) * 2;

    if (target_was_used)
        *target_was_used = false;
    if (request_was_used)
        *request_was_used = false;

    //Can the message fill an active request ?
    if ((myActiveRequests[channel].empty() ) &&
        (myActiveRequests[myNumSockets].empty())    )
    {
        //No active request present
        if (target_buffer == NULL)
        {
            void *tempBuffer = malloc (size);
            sentBytes = ::blocking_recv (mySockets[channel], tempBuffer, size);
            assert (sentBytes == size);

            myOutstandingMsgs[channel].push_back (std::make_pair(size,tempBuffer));
        }
        else
        {
            sentBytes = ::blocking_recv (mySockets[channel], target_buffer, size);
            assert (sentBytes == size);

            if (target_was_used)
                *target_was_used = true;
        }
    }
    else
    {
        Request r;

        //Satisfy an active request
        if (!myActiveRequests[channel].empty() &&
                myActiveRequests[myNumSockets].empty())
        {
            r = myActiveRequests[channel].front();
            myActiveRequests[channel].pop_front();
        }
        else
            if (myActiveRequests[channel].empty() &&
                    !myActiveRequests[myNumSockets].empty())
            {
                r = myActiveRequests[myNumSockets].front();
                myActiveRequests[myNumSockets].pop_front();
            }
            else
                if (myActiveRequests[channel].front().second.first <
                    myActiveRequests[myNumSockets].front().second.first)
                {
                    r = myActiveRequests[channel].front();
                    myActiveRequests[channel].pop_front();
                }
                else
                {
                    r = myActiveRequests[myNumSockets].front();
                    myActiveRequests[myNumSockets].pop_front();
                }

        assert (r.second.second.second >= size);
        sentBytes = ::blocking_recv (mySockets[channel], r.second.second.first, size);
        assert (sentBytes == size);

        if (request_finished)
            *request_finished = r.first;

        if (request_was_used)
            *request_was_used = true;

        myCompletedRequests.insert (
                std::make_pair (r.first,
                        std::make_pair(channel, size)
                )
        );
    }

    return GTI_SUCCESS;
}

/*EOF*/
