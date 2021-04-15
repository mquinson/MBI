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
 * @file MatchExplorer.h
 *       @see must::MatchExplorer.
 *
 *  @date 30.11.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#ifndef MATCHEXPLORER_H
#define MATCHEXPLORER_H

#include <map>
#include <iostream>

namespace must
{
    struct ExplorerLevel
    {
        int currMatch;
        int numAlternatives;
    };

    /**
     * An explorer for choices in wildcard receive matching.
     * It maintains a stack of which alternatives are available for multiple
     * wildcard receives. The number of alternatives grows exponential,
     * so beware of complexity.
     */
    class MatchExplorer
    {
    public:
        /**
         * Initializes the exploration of wildcard matches.
         * Starts with an empty stack with 0 levels.
         */
        MatchExplorer (void);

        /**
         * Destructor.
         */
        ~MatchExplorer (void);

        /**
         * Returns the current stack level.
         * Returns -1 if no stack levels are allocated
         * (after initialization).
         */
        int getCurrentLevel (void);

        /**
         * Returns true if the exploration has information on the current level.
         */
        bool isKnownLevel (void);

        /**
         * Adds a level to the stack.
         * Assumes the first of these alternatives is the current alternative for this level and
         * increases the current level to this level.
         * (subsequent call to  getNextAlternativeId will return the next alternative)
         */
        void addLevel (int numAlternatives);

        /**
         * Returns the index of the next alternative to explore of the current level.
         * @return 0 < index < NumAlternatives[currLevel] for valid stack levels, -1 otherwise.
         */
        int getCurrAlternativeIndex (void);

        /**
         * Increases current stack level by 1.
         */
        void push (void);

        /**
         * Rewinds and increases:
         * a) Increases currAlternative on currLevel
         * -- If this was the last alternative the level is removed and currAlternative of the preceding level is increases
         * b) currLevel is set to 0
         *
         * @return true if a further exploration is available, false otherwise.
         */
        bool nextExploration (void);

        /**
         * Rewinds:
         * b) currLevel is set to 0
         */
        void rewindExploration (void);

        /**
         * Prints the explorer to the given stream.
         * @param out stream to use.
         */
        void print (std::ostream &out);

    protected:
        std::map<int, ExplorerLevel> myStack; //Maps stacklevel to information for this level
        int myCurrLevel;
    };

} /*namespace must*/

#endif /*MATCHEXPLORER_H*/
