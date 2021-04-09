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
 * @file LocationInfo.h
 *       Struct and helpers for location id information.
 *
 *  @date 24.04.2014
 *  @author Tobias Hilbrich
 */

#include "mustConfig.h"
#include "BaseIds.h"

#ifndef LOCATIONINFO_H
#define LOCATIONINFO_H

namespace must
{
    /**
     * Information for a location id.
     */
    struct LocationInfo
    {
        std::string callName; /**< Name of the call that created the record.*/
#ifdef USE_CALLPATH
        std::list<MustStackLevelInfo> stack;
#endif
    };

#ifdef USE_CALLPATH
    inline bool operator<(const LocationInfo& a, const LocationInfo& b)
    {
        //Criteria A: callName
        if (a.callName < b.callName)
            return true;

        if (a.callName != b.callName)
            return false;

        //Criteria B: stack depth
        if (a.stack.size() < b.stack.size())
            return true;

        if (a.stack.size() != b.stack.size())
            return false;

        //Critieria C: stack->symName && lineOffset
        std::list<MustStackLevelInfo>::const_iterator aIter, bIter;
        for (aIter = a.stack.begin(), bIter = b.stack.begin(); aIter != a.stack.end(); aIter++, bIter++)
        {
            if (aIter->symName < bIter->symName)
                return true;
            if (aIter->symName != bIter->symName)
                return false;

            if (aIter->lineOffset < bIter->lineOffset)
                return true;
            if (aIter->lineOffset != bIter->lineOffset)
                return false;
        }

        //We consider this equal if all stack symbol names and the call name are equal
        //(We do not look at file or offsets or module or line, this should usually be ok)
        return false;
    }
#endif

} /*namespace must*/

#endif /*LOCATIONINFO_H*/
