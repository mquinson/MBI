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
 * @file I_FloodControl.h
 *       @see I_FloodControl.
 *
 *  @date 16.07.2012
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze
 */

#include "I_Module.h"
#include "GtiEnums.h"

#ifndef I_FLOODCONTROL_H
#define I_FLOODCONTROL_H

/**
 * Gathers information on the result of processing events.
 * Provides feedback on which communication channels are
 * promising for receiving new events.
 *
 * Dependencies (order as listed):
 * - X
 *
 */
class I_FloodControl : public gti::I_Module
{
public:

    /**
     * Initializes the flood control,
     * any other call that comes before this call is ignored.
     *
     * @param numDownChannels number of channels
     *              connected to the downwards strategy.
     * @param hasIntra true if intra communication is
     *               available on this layer.
     * @param numIntraChannels number of channels
     *               available for intra-layer communication.
     * @param hasUp true if upwards communication is available.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN init (
            unsigned int numDownChannels,
            bool hasIntra,
            unsigned int numIntraChannels,
            bool hasUp) = 0;

    /**
     * Sets where the current record is from.
     *
     * @param direction communication from which this record is.
     * @param channel from which this record is.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN setCurrentRecordInfo (gti::GTI_STRATEGY_TYPE direction, unsigned int channel) = 0;

    /**
     * Returns where the current record is from.
     *
     * @param outDirection communication from which this record is.
     * @param outChannel from which this record is.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN getCurrentRecordInfo (gti::GTI_STRATEGY_TYPE *outDirection, unsigned int *outChannel) = 0;

    /**
     * Marks that the current record could not be processed satisfactorily, i.e., it was
     * buffered in some analysis.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN markCurrentRecordBad (void) = 0;

    /**
     * Retrieves the next decision to fire a test. Returns the direction to receive from and the channel to use.
     * For intra-layer communication and upwards communication, the channel argument should be ignored,
     * the caller should use any up-strategy or any intra channel in this case. (Rational: too many intra communication
     * channels may exist, while multiple up channels should be very rare)
     * @param outDirection to use.
     * @param outChannel to use.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN getCurrentTestDecision (gti::GTI_STRATEGY_TYPE *outDirection, unsigned int *outChannel) = 0;

    /**
     * Notifies that the last decision was not succesful and that the next call to getCurrentTestDecision should return the next
     * best decision.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN nextTestDecision (void) = 0;

    /**
     * Notifies that the last decision was succesful and that the next call to getCurrentTestDecision should restart with the
     * best available decision.
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN rewindDecision (void) = 0;

    /**
     * Returns the maximum badness that this tool node perceived.
     */
    virtual uint64_t getMaxBadness (void) = 0;

    /**
     * Modifies the queue size (number of records received from the direction and channel that
     * can somehow not be processed currently).
     * @param direction of the record whose queue size is modified.
     * @param channel of the record.
     * @param diff that is added to the queue size (use negative values to reduce the queue size).
     * @return see gti::GTI_ANALYSIS_RETURN.
     */
    virtual gti::GTI_ANALYSIS_RETURN modifyQueueSize (gti::GTI_STRATEGY_TYPE direction, unsigned int channel, int diff) = 0;

};/*class I_FloodControl*/

#endif /*I_FLOODCONTROL_H*/
