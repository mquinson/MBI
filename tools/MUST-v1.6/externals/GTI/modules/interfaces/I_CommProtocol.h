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
 * @file I_CommProtocol.h
 *       @see I_CommProtocol.
 *
 * @author Tobias Hilbrich
 */

#include "GtiDefines.h"
#include "GtiEnums.h"
#include "GtiTypes.h"
#include "I_Module.h"

#ifndef I_COMM_PROTOCOL_H
#define I_COMM_PROTOCOL_H

namespace gti
{

/**
 * Class that describes the communication protocol interface.
 *
 * Important, for startup of a communication protocol, the
 * provided module configuration string has to include certain
 * data fields. These are:
 * - \b comm_id - integer, an identifier for a pair of connected
 *            communication protocols (both connected protocols
 *            -- on the upper and lower side -- should use the same
 *            number)
 * - \b side - either "t" or "b", describes whether the comm
 *            protocol is part of the [T]op part of the comm
 *            tree or the [B]ottom part
 * - \b tier_size - integer, the number of processes in this tier of the
 *              communication tree
 * - \b target_tier_size - integer, the number of processes in the target tier
 *              of the communication tree
 * - \b is_intra - integer, either "0" or "1", if not given the default is
 *                       inter communication (between layers), if is_intra=1,
 *                       then this is an intra communication. For intra
 *                       communication, intra communication protocols
 *                       with equal comm_ids will be connected, side is
 *                       of no importance in that case.
 *
 * Further fields will usually be required to initialize a
 * communication protocol. These will usually be implementation
 * dependent, predefined ones /:
 * - \b id - integer, the ID of this tool place within 0-TierSize, should
 *        usually be provided by the tool place main module
 *   TODO: works currently only with mpi-applications
 *
 * A communication protocol is initialized when its constructor is
 * called. For some implementations this may be too early, e.g.
 * a MPI based implementation will have to wait for an MPI_Init
 * call before creating a connection. A good implementation should
 * not fail in such a scenario but rather return a GTI_ERROR_NOT_INITIALIZED
 * for all calls to the protocol. It should try to connect when the
 * waited for event occurs. (Implementations
 * should provide their own mechanic to capture such an event or
 * test for it whenever one of its calls is issued.) Also a communication
 * protocol may issue an "early" shutdown if the communication mechanism
 * becomes unavailable before its shutdown function is called. A communication
 * protocol may try to catch events that will severe its connection and perform
 * an early shutdown in such a case. Also for such a situation all subsequent
 * calls to the communication mechanism will return an
 * GTI_ERROR_NOT_INITIALIZED.
 * Communication strategies may querry for the connection state of a
 * protocol with the following three calls:
 * - gti::I_CommProtocol::isConnected
 * - gti::I_CommProtocol::isInitialized
 * - gti::I_CommProtocol::isFinalized
 *
 *  @todo The interface offers some features that may not be
 *  available for all implementations. A definition needs
 *  to be made to denote where the information on available
 *  features of an implementation is available and how calls
 *  to unsupported features should be handled (e.g. ignore
 *  or abort).
 *
 *  Other maybe helpful calls:
 *  - probe -- is incoming message outstanding - possible with irecv and test ?
 */
class I_CommProtocol : public I_Module
{
public:

    /**
     * Virtual destructor.
     */
    virtual ~I_CommProtocol () {}

    /**
     * Returns true if this communication protocol is connected
     * with its communication partners.
     * Equivalent to :
     *@code
    	 return isInitialized() && !isFinalized();
     *@endcode
     * @return true if connected, false otherwise.
     */
    virtual bool isConnected (void) = 0;

    /**
     * Returns true if this communication protocol was initialized.
     * This may immediately hold after creating a communication
     * protocol or until a certain condition was meet. E.g. a
     * connection server becomes available or a required API was
     * initialized.
     * @return true if initialized, false otherwise.
     */
    virtual bool isInitialized (void) = 0;

    /**
     * Returns true if this communication protocol was finalized and
     * thus disconnected.
     * A protocol may be disconnected before its shutdown function
     * was called. E.g. if its connection was cut or failed.
     * @return true if finalized.
     */
    virtual bool isFinalized (void) = 0;

    /**
     * Shuts the communication protocol down and
     * disconnects.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN shutdown (void) = 0;

    /**
     * The callback `fun` is called, whenever a new client connected
     *
     * @param fun provides a function pointer to be called when new clients show up
     * @param isUsed returns information whether the callback will be called at any time
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN registerNewClientCallback (void(*fun)(void), bool& isUsed) {isUsed=false; return GTI_SUCCESS;}

    /**
     * Returns the number of channels used by
     * this comm protocol.
     * Upwards directed protocols will have one
     * channel, whereas downwards directed ones
     * will usually have more.
     *
     * Intra-communication protocols have as many channels
     * as they have places within the tool layer.
     *
     * @param out_numChannels number of channels is
     *        written to this address.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN getNumChannels (uint64_t* out_numChannels) = 0;

    /**
     * Returns the number of clients used by
     * this comm protocol for shutdown.
     * Upwards directed protocols will have one
     * channel, whereas downwards directed ones
     * will usually have more.
     *
     * Intra-communication protocols have as many clients
     * as they have places within the tool layer.
     *
     * @param out_numClients number of channels is
     *        written to this address.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN getNumClients (uint64_t* out_numClients) {return getNumChannels(out_numClients);}

    /**
     * Returns the id of this place within its layer, a value between
     * 0 and tier-size-1.
     *
     * @param outPlaceId id of place.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN getPlaceId (uint64_t* outPlaceId) = 0;

    /**
     * Removes outstanding requests.
     * Should be used for shutdown synchronization of
     * comm strategies. Issue is that outstanding requests
     * e.g., from a test would eat up synchronization msgs
     * to avoid this outstanding requests can be removed
     * with this call.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN removeOutstandingRequests (void) = 0;

    /**
     * Sends a message synchronized.
     * The call returns after the message arrived at
     * the receiver.
     *
     * The send uses broadcast semantics if more than
     * one receiver is present.
     *
     * @param buf data to be sent.
     * @param num_bytes total size of the message.
     * @param channel number of the channel to send to.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN ssend (
            void* buf,
            uint64_t num_bytes,
            uint64_t channel
    ) = 0;

    /**
     * Sends a non-blocking message.
     * The call returns even if the message
     * was not yet received.
     *
     * The send uses broadcast semantics if more than
     * one receiver is present.
     *
     * @param buf data to be sent.
     * @param num_bytes total size of the message.
     * @param out_request token used to complete the
     *                communication.
     * @param channel number of the channel to send to.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early, and GTI_ERROR_OUTSTANDING_LIMIT
     *         if the send was not started due to an excessive amount of
     *         outstanding sends.
     *
     * @see test
     * @see wait
     */
    virtual GTI_RETURN isend (
            void* buf,
            uint64_t num_bytes,
            unsigned int *out_request,
            uint64_t channel
    ) = 0;

    /**
     * Synchronized receive.
     * The call returns after the message was received.
     * @param out_buf memory buffer to hold the data.
     * @param num_bytes allocated size of the buffer.
     * @param out_length the number of bytes received.
     * @param channel number of the channel to receive from or
     *        RECV_ANY_CHANNEL for an arbitrary channel.
     * @param out_channel the number of bytes received.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN recv (
            void* out_buf,
            uint64_t num_bytes,
            uint64_t *out_length,
            uint64_t channel,
            uint64_t *out_channel
    ) = 0;

    /**
     * Non-blocking receive.
     * The call returns even if no message could be
     * received yet.
     * @param out_buf memory buffer to hold the data.
     * @param num_bytes allocated size of the buffer.
     * @param out_request token used to complete the
     *                communication.
     * @param channel number of the channel to receive from or
     *        RECV_ANY_CHANNEL for an arbitrary channel.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN irecv (
            void* out_buf,
            uint64_t num_bytes,
            unsigned int *out_request,
            uint64_t channel
    ) = 0;

    /**
     * Tests for completion of a non-blocking communication.
     *
     * For non-blocking receives, the out_receive_length argument
     * is used to return the number of bytes received. This argument
     * is not updated for completion of non-blocking send calls.
     *
     * @param request the token associated with the communication.
     * @param out_completed set to true if the communication was
     *        successful.
     * @param out_receive_length set the number of bytes
     *        received, if the communication was completed.
     * @param out_channel output: pointer to storage for a long value
     *        which is set to the id of the cannel from which the
     *        test was successful, only set if out_completed was set
     *        to true.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN test_msg (
            unsigned int request,
            int* out_completed,
            uint64_t* out_receive_length,
            uint64_t* out_channel
    ) = 0;

    /**
     * Waits for completion of a non-blocking communication.
     *
     * For non-blocking receives, the out_receive_length argument
     * is used to return the number of bytes received. This argument
     * is not updated for completion of non-blocking send calls.
     *
     * @param request the token associated with the communication.
     * @param out_receive_length set the number of bytes
     *        received.
     * @param out_channel output: pointer to storage for a long value
     *        which is set to the id of the cannel from which the
     *        test was successful.
     * @return GTI_SUCCESS if successful, GTI_ERROR for an unspecified
     *         error, GTI_ERROR_NOT_INITIALIZED if the protocol was never
     *         connected or was shutdown early.
     */
    virtual GTI_RETURN wait_msg (
            unsigned int request,
            uint64_t* out_receive_length,
            uint64_t* out_channel
    ) = 0;
}; /*class I_CommProtocol*/
} /*namespace gti*/

#endif /* I_COMM_PROTOCOL_H */
