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
 * @file MessageReduction.h
 *       @see MUST::MessageReduction.
 *
 *  @date 05.08.2013
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"
#include "I_FinishNotify.h"
#include "BaseApi.h"

#include "I_MessageReduction.h"

#ifndef MESSAGEREDUCTION_H
#define MESSAGEREDUCTION_H

using namespace gti;

namespace must
{
    /**
     * Storage class for message reduction
     */
    class MessageRepresentation
    {
    protected:
        int myMsgId;
        MustParallelId myPId;
        MustLocationId myLId;
        std::string myCallName;
        int myMsgType;
        std::string myText;
        int myNumReferences;
        MustParallelId *myRefPIds;
        MustLocationId *myRefLIds;

        std::map<int, std::pair<int, int> > myStrides; //Maps startRank, to (stride,count)

    public:

        /**
         * Copy constructor.
         */
        MessageRepresentation( const MessageRepresentation& other);

        /*
         * Creates a representation from the given information.
         * Copies the arrays into own memory.
         */
        MessageRepresentation (
                int msgId,
                uint64_t pIdRef,
                uint64_t lIdRef,
                std::string callName,
                int startRank,
                int stride,
                int count,
                int msgType,
                char *text,
                int textLen,
                int numReferences,
                uint64_t* refPIds,
                uint64_t* refLIds);

        /**
         * Destructor
         */
        ~MessageRepresentation (void);

        /**
         * Checks whether the given new message belongs into this representation.
         */
        bool belongsToRepresentation (
                int msgId,
                std::string callName,
                int msgType,
                std::string text,
                int startRank,
                int stride,
                int count
                );

        /**
         * Add to representation.
         */
        void addToRepresentation (
                int startRank,
                int stride,
                int count);

        /**
         * Creates event(s) from this representation.
         */
        void forwardRepresentation (handleNewMessageReducedP fNewMsg);
    };


	/**
     * Implementation of I_MessageReduction.
     */
    class MessageReduction : public gti::ModuleBase<MessageReduction, I_MessageReduction>, I_FinishListener
    {
    public:
			/**
			 * Constructor.
			 * @param instanceName name of this module instance.
			 */
            MessageReduction (const char* instanceName);

			/**
			 * Destructor.
			 */
			virtual ~MessageReduction (void);

			/**
			 * @see I_MessageReduction::reduce.
			 */
			GTI_ANALYSIS_RETURN reduce (
			        int msgId,
			        int hasLocation,
			        uint64_t pId,
			        uint64_t lId,
			        int msgType,
			        char *text,
			        int textLen,
			        int numReferences,
			        uint64_t* refPIds,
			        uint64_t* refLIds,
			        gti::I_ChannelId *thisChannel,
			        std::list<gti::I_ChannelId*> *outFinishedChannels
			);

			/**
			 * @see I_MessageReduction::reduceStrided
			 */
			GTI_ANALYSIS_RETURN reduceStrided (
			        int msgId,
			        uint64_t pIdRef,
			        uint64_t lIdRef,
			        int startRank,
			        int stride,
			        int count,
			        int msgType,
			        char *text,
			        int textLen,
			        int numReferences,
			        uint64_t* refPIds,
			        uint64_t* refLIds,
			        gti::I_ChannelId *thisChannel,
			        std::list<gti::I_ChannelId*> *outFinishedChannels
			);

			/**
			 * @see I_FinishListener::finish
			 */
			void finish (void);

			/**
			 * The timeout function, see gti::I_Reduction.timeout
			 */
			void timeout (void);

    protected:
			I_ParallelIdAnalysis *myPIdModule;
			I_LocationAnalysis *myLIdModule;
			I_FinishNotify *myNotify;

			handleNewMessageReducedP myIntroduceMessage;

			std::list<MessageRepresentation> myReps;

			bool myGotFinish;

    }; /*class MessageReduction */
} /*namespace MUST*/

#endif /*MESSAGEREDUCTION_H*/
