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
 * @file SimpleTcpServer.cpp
 * A TCP server that establishes connections between
 * processes. Uses the protocol from cProtSimpleTcp.
 * @see CProtSimpleTCP
 * @author Tobias Hilbrich
 * @date 23.04.2009
 */

#include "SimpleTcpServer.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * Function to perform a blocking receive with sockets.
 */
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

/**
 * Function to perform a blocking send with sockets.
 */
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

/* ==== BUNDLE ====
 * Participant             myParticipantT;
 * std::list<Participant>  myParticipantsB;
 */

Bundle::Bundle (Participant t, std::list<Participant> bList)
    : myParticipantT (t), myParticipantsB (bList)
{
    /*Nothing to do*/
}

/* ==== PARTICIPANT ====
 * unsigned int    mySide;
 * uint64_t   myTierID;
 * uint64_t   myIP;
 * uint64_t   myPort;
 * int             mySocket;
 */
Participant::Participant (void)
{
    mySocket = -1;
}

Participant::Participant (unsigned int side, uint64_t tierID,
                 uint64_t selfID, uint64_t tierSize,
                 uint64_t IP, uint64_t port,
                 int socket)
    : mySide(side), myTierID(tierID), mySelfID(selfID), myIP(IP),
      myPort(port), myTierSize(tierSize), mySocket(socket)
{
    //DEBUG:
    std::cout
        << "New participant: {side="
        << mySide
        << ", tierID="
        << myTierID
        << ", selfID="
        << mySelfID
        << ", ip="
        << myIP
        << ", port="
        << myPort
        << ", tierSize="
        << myTierSize
        << ", socket="
        << mySocket
        << "}"
        << std::endl;
}

void Participant::closeSocket (void)
{
    if (mySocket >= 0)
        ::close (mySocket);
}

void Participant::printParticipant (void)
{
    std::cout
        << "("
        << "mySide:"
        << mySide
        << ", myTierID:"
        << myTierID
        << ", myIP"
        << myIP
        << ", myPort"
        << myPort
        << ", myTierSize"
        << myTierSize
        << ", mySocket"
        << mySocket
        << ")";
}


/* ==== TIER ====
 * bool bTierSizeTKnown;
 * bool bTierSizeBKnown;
 * uint64_t myTierID;
 * uint64_t myTierSizeT;
 * uint64_t myTierSizeB;
 * std::map <uint64_t, Participant> myParticipantsT;
 * std::map <uint64_t, Participant> myParticipantsB;
 *
 */
bool          Tier::mySetConnectPort = false;
uint64_t Tier::myConnectPort = 0;

Tier::Tier (uint64_t myTierID, uint64_t connectPort)
    : myTierID (myTierID)
{
    myParticipantsT.clear ();
    myParticipantsB.clear ();
    bTierSizeTKnown = false;
    bTierSizeBKnown = false;

    if (!mySetConnectPort)
    {
        myConnectPort = connectPort;
        mySetConnectPort = true;
    }
}

Tier::~Tier (void)
{
    std::map <uint64_t, Participant>::iterator iter;

    //close the sockets of all participants
    for (iter = myParticipantsT.begin(); iter != myParticipantsT.end(); iter++)
        iter->second.closeSocket();

    for (iter = myParticipantsB.begin(); iter != myParticipantsB.end(); iter++)
        iter->second.closeSocket();

    myParticipantsT.clear();
    myParticipantsB.clear();
    bTierSizeTKnown = false;
    bTierSizeBKnown = false;
}

bool Tier::addParticipant (Participant newParticipant)
{
    assert ((newParticipant.mySide == 1) ||
            (newParticipant.mySide == 0)   );

    //Insert new participant
    if (newParticipant.mySide == 0)
    {
        //Side is Top
        if (!bTierSizeTKnown)
        {
            myTierSizeT = newParticipant.myTierSize;
            bTierSizeTKnown = true;
        }
        else
        {
            assert (newParticipant.myTierSize == myTierSizeT);
        }

        myParticipantsT.insert (
                std::make_pair (newParticipant.mySelfID,newParticipant)
                );
    }
    else
    {
        //Side it bottom
        if (!bTierSizeBKnown)
        {
            myTierSizeB = newParticipant.myTierSize;
            bTierSizeBKnown = true;
        }
        else
        {
            assert (newParticipant.myTierSize == myTierSizeB);
        }

        myParticipantsB.insert (
                std::make_pair (newParticipant.mySelfID,newParticipant)
        );
    }

    //Check whether all participants have arrived
    if (checkForTierCompletness())
    {
        //partion, print, and send out bundle info
        std::list<Bundle> bundles;
        bundles = partitionTier ();
        printBundles (bundles);
        sendBundles (bundles);
        return true;
    }

    //not yet completed ...
    return false;
}

void Tier::printTier (void)
{
    std::map <uint64_t, Participant>::iterator iter;

    std::cout
        << "==== Tier: "
        << myTierID
        << " ===="
        << std::endl
        << "Top layer:"
        << std::endl;

    //close the sockets of all participants
    for (iter = myParticipantsT.begin(); iter != myParticipantsT.end(); iter++)
        iter->second.printParticipant();

    std::cout
            << std::endl
            << "Bottom layer:"
            << std::endl;

    for (iter = myParticipantsB.begin(); iter != myParticipantsB.end(); iter++)
        iter->second.printParticipant();
}

bool Tier::checkForTierCompletness (void)
{
    //Check whether all participants have arrived
    if (bTierSizeTKnown && bTierSizeBKnown     &&
       (myParticipantsT.size() == myTierSizeT) &&
       (myParticipantsB.size() == myTierSizeB)    )
        return true;

    return false;
}

std::list<Bundle> Tier::partitionTier (void)
{
    //Top layer must be smaller or equal in size
    assert (myTierSizeT <= myTierSizeB);

    uint64_t avgBProcsPerTProc;
    uint64_t processesLeft;
    uint64_t numProcsCurrentT;
    uint64_t nextBProc = 0;
    Participant   currT;
    std::list<Participant> currBList;
    std::list<Bundle> ret;

    ret.clear();
    avgBProcsPerTProc = myTierSizeB / myTierSizeT;
    processesLeft = myTierSizeB - avgBProcsPerTProc*myTierSizeT;

    assert (processesLeft <= myTierSizeT); //math should be with us ;)

    for (uint64_t i = 0; i < myTierSizeT; i++)
    {
        numProcsCurrentT = avgBProcsPerTProc;
        if (processesLeft)
        {
            numProcsCurrentT++;
            processesLeft--;
        }


        currT = myParticipantsT[i];
        currBList.clear ();

        for (uint64_t j = nextBProc; j < nextBProc + numProcsCurrentT;j++)
        {
            currBList.push_back (myParticipantsB[j]);
        }
        nextBProc += numProcsCurrentT;

        ret.push_back (Bundle (currT, currBList));
    }

    return ret;
}

void Tier::sendBundles (std::list<Bundle> bundles)
{
    uint64_t buffNumBundles;
    uint64_t *buffBundleInfo;
    uint64_t lenNum, lenInfo;
    std::list<Bundle>::iterator iter;
    std::list<Participant>::iterator bIter;
    uint64_t currConnectPort;
    uint64_t currPort = myConnectPort;
    int ret;

    for (iter = bundles.begin(); iter != bundles.end(); iter++)
    {
        //(1) Send num bundles top client
        //  1 x uint64_t
        buffNumBundles = iter->myParticipantsB.size();
        lenNum = sizeof (uint64_t);

        ret = all_send(iter->myParticipantT.mySocket, &buffNumBundles, lenNum);
        assert (ret == lenNum);

        //(2) Send connection partners to top client
        //  #NumPartners * (uint64_t, uint64_t)  (IP, Port)
        buffBundleInfo = new uint64_t[2*iter->myParticipantsB.size()];
        lenInfo = sizeof (uint64_t) * 2 * iter->myParticipantsB.size();

        int i = 0;
        currConnectPort = currPort;
        for (bIter = iter->myParticipantsB.begin(); bIter != iter->myParticipantsB.end(); bIter++, i++)
        {
            buffBundleInfo[2*i] = bIter->myIP;
            buffBundleInfo[2*i+1] = currConnectPort;

            currConnectPort++;
        }

        ret = all_send(iter->myParticipantT.mySocket, buffBundleInfo, lenInfo);
        assert (ret == lenInfo);

        //(3) Loop over all Bottom clients
        buffNumBundles = 1;
        lenInfo = sizeof (uint64_t) * 2;
        currConnectPort = currPort;
        for (bIter = iter->myParticipantsB.begin(); bIter != iter->myParticipantsB.end(); bIter++)
        {
            //(3a) Send num bundles and connection info to bottom client
        		ret = all_send(bIter->mySocket, &buffNumBundles, lenNum);
            assert (ret == lenNum);

            buffBundleInfo[0] = iter->myParticipantT.myIP;
            buffBundleInfo[1] = currConnectPort;

            currConnectPort++;

            ret = all_send(bIter->mySocket, buffBundleInfo, lenInfo);
            assert (ret == lenInfo);

            //Receive "I-am-listening" from bottom client
            ret = blocking_recv(bIter->mySocket, buffBundleInfo, sizeof (uint64_t));
            assert (ret == sizeof (uint64_t));

            //Send "good-to-connect" to top client
            ret = all_send(iter->myParticipantT.mySocket, buffBundleInfo, sizeof (uint64_t));
            assert (ret == sizeof (uint64_t));

            //(3b) Receive connected reply from client
            ret = blocking_recv(bIter->mySocket, buffBundleInfo, sizeof (uint64_t));
            assert (ret == sizeof (uint64_t));
        }

        currPort = currConnectPort;
        myConnectPort = currConnectPort;

        //(4) Receive connected reply from top client
        ret = blocking_recv(iter->myParticipantT.mySocket, buffBundleInfo, sizeof (uint64_t));
        assert (ret == sizeof (uint64_t));

        delete (buffBundleInfo);
    }//for bundles
}

void Tier::printBundles (std::list<Bundle> bundles)
{
    std::list<Bundle>::iterator iter;
    for (iter = bundles.begin(); iter != bundles.end(); iter++)
    {
        std::cout
            << "Connection bundle: T="
            << iter->myParticipantT.mySelfID
            << " B={";

        std::list<Participant>::iterator bIter;
        for (bIter = iter->myParticipantsB.begin(); bIter != iter->myParticipantsB.end(); bIter++)
        {
            if (bIter != iter->myParticipantsB.begin())
                std::cout << ", ";

            std::cout
                << bIter->mySelfID;
        }

        std::cout << "}" << std::endl;
    }
}

/* ==== SERVER ====
 * std::map <uint64_t, Tier> myTiers;
 * int                 myServerSocket;
 * uint64_t       myIP;
 * uint64_t       myPort;
 */
Server::Server (uint64_t IP, uint64_t listenPort, uint64_t connectPort)
    : myPort (listenPort), myConnectPort(connectPort), myIP (IP)
{
    myTerminate = false;
    myTiers.clear ();
    setupServerSocket ();
    acceptConnections ();
}

Server::~Server (void)
{
    close (myServerSocket);
}

void Server::setupServerSocket (void)
{
    struct sockaddr_in serverAddr;
    int ret;

    /* Create the TCP socket */
    myServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert (myServerSocket >= 0);

    /* Construct the server sockaddr_in structure */
    memset(&serverAddr, 0, sizeof(serverAddr));       /* Clear struct */
    serverAddr.sin_family = AF_INET;                  /* Internet/IP */
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
    serverAddr.sin_port = (uint16_t) myPort;          /* server port */

    /* Bind the server socket */
    ret = bind(myServerSocket, (struct sockaddr *) &serverAddr,
            sizeof(serverAddr));
    assert (ret >= 0);

    /* Listen on the server socket */
    ret = listen(myServerSocket, 10000);
    assert (ret >= 0);

    /* Set socket non-blocking */
    long save_fd = fcntl( myServerSocket, F_GETFL );
    save_fd |= O_NONBLOCK;
    ret = fcntl( myServerSocket, F_SETFL, save_fd );
    assert (ret >= 0);

    /* Give some output */
    std::cout
        << "Started to listen for connections ..."
        << std::endl;
}

void Server::acceptConnections (void)
{
    int clientSock;
    struct sockaddr_in clientAddr;

    /* Run until cancelled */
    while (!myTerminate) {
        unsigned int clientlen = sizeof(clientAddr);

        /* Wait for client connection */
        clientSock = accept(myServerSocket, (struct sockaddr *) &clientAddr,
                    &clientlen);

        if (clientSock == -1)
        {
            assert (errno == EAGAIN);
            usleep(100);
            continue;
        }

        assert (clientSock >= 0);

        std::cout
            << "Client connected: "
            << inet_ntoa(clientAddr.sin_addr)
            << std::endl;

        /*Recieve the info header from the client*/
        uint64_t connectionInfoBuffer[4];
        unsigned int len = sizeof(uint64_t) * 4;

        uint64_t ret_len = blocking_recv(clientSock, connectionInfoBuffer, len);
        assert (ret_len == len);

        //create participant
        Participant p (connectionInfoBuffer[1], connectionInfoBuffer[0],
                       connectionInfoBuffer[3], connectionInfoBuffer[2],
                       clientAddr.sin_addr.s_addr, (uint64_t) clientAddr.sin_port,
                       clientSock);

        //add participant to tier (add/delete tier if necessary)
        std::map <uint64_t, Tier>::iterator iter;
        iter = myTiers.find (p.myTierID);
        if (iter == myTiers.end())
        {
            myTiers.insert (std::make_pair(p.myTierID,Tier (p.myTierID,myConnectPort)));
            iter = myTiers.find (p.myTierID);
        }

        bool tierDone = myTiers.find(p.myTierID)->second.addParticipant (p);

        if (tierDone)
            myTiers.erase (iter);
    }
}

/* ==== MAIN & Base stuff ====
 */
Server *pServer = NULL;

void termination_handler (int signum)
{
    std::cerr << "Got SIGTERM, shutting down!" << std::endl;
    pServer->myTerminate = true;
}

int main (int argc, char** argv)
{

    if (argc != 3)
    {
        std::cout << "Usage: "
                  << argv[0]
                  << " <Port1> <Port2>"
                  << std::endl
                  << "Where <Port1> is the port to listen for processes and"
                  << "<Port2> is the port used to connect processes."
                  << std::endl;
        return 1;
    }

    uint64_t port = atol (argv[1]);
    uint64_t port2 = atol (argv[2]);
    struct hostent *host;     /* host information */
    struct in_addr haddr;    /* internet address */

    if (getenv("GTI_TCP_SERVER_IP") != NULL)
    {
        host = gethostbyname(getenv("GTI_TCP_SERVER_IP"));
    }
    else
    {
        host = gethostbyname("localhost");
    }

    if (host == NULL)
    {
        std::cerr
            << "Failed to look up IP of localhost!"
            <<  std::endl;

        return 1;
    }

    haddr.s_addr = *((uint64_t *) host->h_addr_list[0]);
    std::cout
        << "Server IP is: "
        << inet_ntoa(haddr)
        << std::endl;

    //Set signal handler
    signal (SIGTERM, termination_handler);

    //Start the actual server
    pServer = new Server (haddr.s_addr, port, port2);

    if (pServer)
        delete (pServer);
    pServer = NULL;

    return 0;
}
