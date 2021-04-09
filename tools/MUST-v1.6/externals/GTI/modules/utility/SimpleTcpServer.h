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
 * @file SimpleTcpServer.h
 * A TCP server that establishes connections between
 * processes. Uses the protocol from cProtSimpleTcp.
 * @see CProtSimpleTCP
 * @author Tobias Hilbrich
 * @date 23.04.2009
 */

#ifndef SIMPLE_TCP_SERVER_H
#define SIMPLE_TCP_SERVER_H

#include <map>
#include <list>
#include <unistd.h>
#include <stdint.h>

/**
 * A participant of a connection.
 */
class Participant
{
public:
    unsigned int    mySide;
    uint64_t   myTierID;
    uint64_t   mySelfID;
    uint64_t   myIP;
    uint64_t   myPort;
    uint64_t   myTierSize;
    int             mySocket;

    Participant (void);
    Participant (unsigned int side, uint64_t tierID,
                 uint64_t selfID, uint64_t tierSize,
                 uint64_t   IP, uint64_t port,
                 int socket);
    void closeSocket (void);
    void printParticipant (void);
};

/**
 * Class for a partition of the connection being established.
 */
class Bundle
{
public:
    Participant             myParticipantT;
    std::list<Participant>  myParticipantsB;

    Bundle (Participant t, std::list<Participant> bList);
};


/**
 * A connection layer.
 */
class Tier
{
protected:
    bool bTierSizeTKnown;
    bool bTierSizeBKnown;

    bool checkForTierCompletness (void);
    std::list<Bundle> partitionTier (void);
    void sendBundles (std::list<Bundle> bundles);
    void printBundles (std::list<Bundle> bundles);

public:
    uint64_t myTierID;
    uint64_t myTierSizeT;
    uint64_t myTierSizeB;
    std::map <uint64_t, Participant> myParticipantsT;
    std::map <uint64_t, Participant> myParticipantsB;
    static bool                mySetConnectPort;
    static uint64_t       myConnectPort;

    Tier (uint64_t myTierID, uint64_t connectPort);
    ~Tier (void);
    /**
     * @return true if this was the last participant of
     *         this tier and the communication between
     *         the tier members was established,
     *         false otherwise.
     */
    bool addParticipant (Participant newParticipant);
    void printTier (void);
};

/**
 * The TCP connection server.
 */
class Server
{
protected:
    std::map <uint64_t, Tier> myTiers;
    int                 myServerSocket;
    uint64_t       myIP;
    uint64_t       myPort;
    uint64_t       myConnectPort;

    void setupServerSocket (void);
    void acceptConnections (void);

public:
    bool                myTerminate;

    Server (uint64_t IP, uint64_t listenPort, uint64_t connectPort);
    ~Server (void);
};

#endif /*SIMPLE_TCP_SERVER_H*/
