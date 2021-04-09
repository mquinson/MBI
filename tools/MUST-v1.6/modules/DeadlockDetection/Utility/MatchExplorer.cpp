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
 * @file MatchExplorer.cpp
 *       @see must::MatchExplorer.
 *
 *  @date 30.11.2011
 *  @author Tobias Hilbrich, Mathias Korepkat, Joachim Protze, Fabian Haensel
 */

#include "MatchExplorer.h"

using namespace must;

//=============================
// Constructor
//=============================
MatchExplorer::MatchExplorer (void)
    : myStack(),
      myCurrLevel (-1)
{
    //Nothing to do
}

//=============================
// Destructor
//=============================
MatchExplorer::~MatchExplorer (void)
{
    myStack.clear();
}

//=============================
// getCurrentLevel
//=============================
int MatchExplorer::getCurrentLevel (void)
{
    return myCurrLevel;
}

//=============================
// isKnownLevel
//=============================
bool MatchExplorer::isKnownLevel (void)
{
    std::map<int, ExplorerLevel>::iterator pos;
    pos = myStack.find (myCurrLevel);

    if (pos == myStack.end())
        return false;

    return true;
}

//=============================
// addLevel
//=============================
void MatchExplorer::addLevel (int numAlternatives)
{
    int nextLevel = 0;

    if (myStack.rbegin() != myStack.rend())
    {
        nextLevel = myStack.rbegin()->first + 1;
    }

    ExplorerLevel level;
    level.currMatch = 0;
    level.numAlternatives = numAlternatives;

    myStack.insert(std::make_pair(nextLevel, level));
    myCurrLevel = nextLevel;
}

//=============================
// getCurrAlternativeIndex
//=============================
int MatchExplorer::getCurrAlternativeIndex (void)
{
    if (myStack.find(myCurrLevel) == myStack.end())
        return -1;

    return myStack[myCurrLevel].currMatch;
}

//=============================
// down
//=============================
void MatchExplorer::push (void)
{
    myCurrLevel++;
}

//=============================
// nextExploration
//=============================
bool MatchExplorer::nextExploration (void)
{
    if (myStack.rbegin() == myStack.rend())
        return false;

    int maxLevel = myStack.rbegin()->first;

    myStack[maxLevel].currMatch = myStack[maxLevel].currMatch + 1;

    if (myStack[maxLevel].currMatch == myStack[maxLevel].numAlternatives)
    {
        //We are done with this level, recurse to preceding level
        myStack.erase(maxLevel);
        return nextExploration ();
    }

    myCurrLevel = 0;

    return true;
}

//=============================
// rewindExploration
//=============================
void MatchExplorer::rewindExploration (void)
{
    myCurrLevel = 0;
}

//=============================
// print
//=============================
void MatchExplorer::print (std::ostream &out)
{
    std::map<int, ExplorerLevel>::iterator iter;

    out << "PRINT OF Exploration stack:";
    for (iter = myStack.begin(); iter != myStack.end(); iter++)
    {
        if (iter != myStack.begin()) out << ".";
        out << iter->second.currMatch << "/" << iter->second.numAlternatives;
    }
    out << std::endl;
}

/*EOF*/
