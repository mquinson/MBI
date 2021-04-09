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
 * @file CProtMpiSplited.h
 *       MPI based communication using the split module (CProtMpiSplitModule.cpp).
 *       MPI_COMM_WORLD is split into multiple disjunct sets of processes, set number 0
 *       is used for the application, the remaining sets are used for levels of tool
 *       processes. This protocol establishes a MPI call based communication between
 *       these process sets.
 *
 * @author Tobias Hilbrich
 * @date 28.07.2009
 */

#include "I_CommProtocol.h"
#include "ModuleBase.h"
#include <list>
#include <map>
#include <vector>

#ifndef CPROT_MPI_SPLITED_H
#define CPROT_MPI_SPLITED_H

namespace gti
{
	/**
	 * Used to store information about an outstanding request.
	 */
	class CProtMPISplitedRequest
	{
	public:
		unsigned int 	id;
		MPI_Request 	mpi_request;
		uint64_t 	num_bytes;
		uint64_t 	channel;
		bool 			isRecv;

		/**
		 * Proper constructor.
		 */
		CProtMPISplitedRequest (
				unsigned int id,
				MPI_Request mpi_request,
				uint64_t num_bytes,
				uint64_t channel,
				bool isRecv);
	};

    /**
     * Implements the communication protocol interface using a MPI based communication.
     * MPI_COMM_WORLD is split into multiple disjunct sets of processes, set number 0
     * is used for the application, the remaining sets are used for levels of tool
     * processes. This protocol establishes a MPI call based communication between
     * these process sets.
     */
    class CProtMPISplited : public ModuleBase<CProtMPISplited, I_CommProtocol>
    {
    protected:

        uint64_t myPlaceId; /**< Own place id.*/

        //Basic state
        bool myIsConnected;
        bool myWasShutdownCalled;

        //Only for inter-communication
        std::vector<int> myPartnerRanks; /**< Ranks of comm. partners. */
        std::map<int, int> myMapRankToChannel; /**< Maps a rank from myPartnerRanks to its corresponding channel. */
        bool myIsTop; /**< True if this is the top layer.*/

        //For both communication types
        bool myIsIntra;
        int myNumChannels;
        MPI_Comm myComm;
        std::map<unsigned int, CProtMPISplitedRequest > myRequests; /**< Maps integer identifiers to MPI requests and an int. The int is either -1 for it is a receive and length has to be querried, or a value >=0 for a send. */
        unsigned int myNextRequestID;
        int myNextRoundRobin;

        /**
         * Starts the connection with the communication partners.
         */
        GTI_RETURN startup (void);

        /**
         * Returns the communication (MPI) rank to use for the given channel.
         * @param channel to translate.
         * @return translation.
         */
        int getRankForChannel (int channel);

        /**
         * Reverse translation of a message received from some rank to its
         * associated channel.
         * @param rank to translate.
         * @return translation.
         */
        int getChannelForRank (int rank);

    public:

        /**
         * Constructor.
         * @ref ModConf - The module configuration syntax
         * @param instanceName name of this module instance.
         */
    	CProtMPISplited (const char* instanceName);

        /**
         * Destructor.
         */
        ~CProtMPISplited (void);

        /**
         * @see gti::I_CommProtocol::isConnected
         */
        bool isConnected (void);

        /**
         * @see gti::I_CommProtocol::isInitialized
         */
        bool isInitialized (void);

        /**
         * @see gti::I_CommProtocol::isFinalized
         */
        bool isFinalized (void);

        /**
         * Function that is called after MPI_Init was called.
         * Should be triggered from a MPI_Init wrapper.
         * Used to start the MPI based connection if MPI
         * was not initialized when the object was created.
         */
        void notifyMpiInit (void);

        /**
         * Function that is called before MPI_Finalize is issued.
         * Should be triggered from a MPI_Finalize wrapper.
         * Used to free the MPI based connection if the
         * communication protocol was not yet shutdown.
         */
        void notifyMpiFinalize (void);

        /**
         * @see I_CommProtocol::getNumChannels
         */
        GTI_RETURN getNumChannels (uint64_t* out_numChannels);

        /**
         * @see gti::I_CommProtocol::getPlaceId
         */
        GTI_RETURN getPlaceId (uint64_t* outPlaceId);

        /**
         * @see I_CommProtocol::removeOutstandingRequests
         */
        GTI_RETURN removeOutstandingRequests (void);

        /**
         * @see I_CommProtocol::shutdown
         */
        GTI_RETURN shutdown (void);

        /**
         * @see I_CommProtocol::ssend
         */
        GTI_RETURN ssend (
                void* buf,
                uint64_t num_bytes,
                uint64_t channel
                );

        /**
         * @see I_CommProtocol::isend
         */
        GTI_RETURN isend (
                void* buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see I_CommProtocol::recv
         */
        GTI_RETURN recv (
                void* out_buf,
                uint64_t num_bytes,
                uint64_t* out_length,
                uint64_t channel,
                uint64_t *out_channel
                );

        /**
         * @see I_CommProtocol::irecv
         */
        GTI_RETURN irecv (
                void* out_buf,
                uint64_t num_bytes,
                unsigned int *out_request,
                uint64_t channel
                );

        /**
         * @see I_CommProtocol::test_msg
         */
        GTI_RETURN test_msg (
                unsigned int request,
                int* out_completed,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );

        /**
         * @see I_CommProtocol::wait_msg
         */
        GTI_RETURN wait_msg (
                unsigned int request,
                uint64_t* out_receive_length,
                uint64_t* out_channel
                );
    }; /*class CProtMPISplited*/
} /*namespace gti*/

#endif /* CPROT_MPI_SPLITED_H */

