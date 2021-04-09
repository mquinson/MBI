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
 * @file I_TrackBase.h
 *       @see I_TrackBase.
 *
 *  @date 12.06.2017
 *  @author Joachim Protze
 */

#ifndef I_TRACKBASE_H
#define I_TRACKBASE_H

namespace must
{
    /**
     * Interface of common basic functions in tracking modules
     */
    template<typename FULL_INFO> //Type of full info, maintained with pointers
    class I_TrackBase
    {
    public:
        /**
         * Test whether a handle is predefined.
         * @return true if provided info is for a predefined handle.
         */
        virtual bool isPredefined (FULL_INFO * info) = 0;
    };
}

#endif /* I_TRACKBASE_H */
