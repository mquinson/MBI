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
 * @file MsgLoggerHtml.h
 *       @see MUST::MsgLoggerHtml.
 *
 *  @date 20.01.2011
 *  @author Tobias Hilbrich
 */

#include "ModuleBase.h"
#include "I_MessageLogger.h"
#include "I_ParallelIdAnalysis.h"
#include "I_LocationAnalysis.h"

#include <fstream>

#ifndef MSGLOGGERHTML_H
#define MSGLOGGERHTML_H

using namespace gti;

namespace must
{
    /**
     * Implementation of I_MessageLogger that writes
     * an HTML file.
     */
    class MsgLoggerHtml : public gti::ModuleBase<MsgLoggerHtml, I_MessageLogger>
    {
    public:
            /**
             * Constructor.
             * @param instanceName name of this module instance.
             */
            MsgLoggerHtml (const char* instanceName);

            /**
             * Destructor.
             */
            virtual ~MsgLoggerHtml (void);

            /**
             * @see I_MessageLogger::log.
             */
            GTI_ANALYSIS_RETURN log (
                    int msgId,
                    int hasLocation,
                    uint64_t pId,
                    uint64_t lId,
                    int msgType,
                    char *text,
                    int textLen,
                    int numReferences,
                    uint64_t* refPIds,
                    uint64_t* refLIds
            );

            /**
             * @see I_MessageLogger::logStrided.
             */
            GTI_ANALYSIS_RETURN logStrided (
                    int msgId,
                    uint64_t pId,
                    uint64_t lId,
                    int startRank,
                    int stride,
                    int count,
                    int msgType,
                    char *text,
                    int textLen,
                    int numReferences,
                    uint64_t* refPIds,
                    uint64_t* refLIds
            );

    protected:

            /**
             * Flushes the file to disk and writes the trailer
             * to it.
             */
            void flush (void);

            /**
             * Removes a possibly printed trailer from the file
             * used when an additional message appeared
             * after the file was already flushed.
             */
            void unflush (void);

            /**
             * Prints the trailer to the file that closes the
             * HTML document.
             * @param finalNotes Print a message (or not) indicating proper and intended tool shut down
             */
            void printTrailer (bool finalNotes = false);

            /**
             * Prints the header for the HTML document.
             */
            void printHeader (void);

            /**
             * Prints a location.
             * @param pId of location
             * @param lId of location
             */
            void printLocation (MustParallelId pId, MustLocationId lId);

            /**
             * Helper that prints an occurrence count nicely formated to fit into our regular outputs.
             * @param out stream to print into.
             * @param lId location id that stores the occurrences count
             */
            void printOccurenceCount (std::ostream& out, MustLocationId lId);

            I_ParallelIdAnalysis *myPIdModule;
            I_LocationAnalysis *myLIdModule;

            std::ofstream myOut;
            bool myPrintedTrailer;
            bool myLineEven;

            bool myLoggedWarnError;
    }; /*class MsgLoggerHtml */
} /*namespace MUST*/

#endif /*MSGLOGGERHTML_H*/
