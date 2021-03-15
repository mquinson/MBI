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
 * @file I_Destructable.h
 *       @see I_Destructable.
 *
 *  @date 27.06.2011
 *  @author Joachim Protze
 */

#ifndef I_DESTRUCTABLE_H
#define I_DESTRUCTABLE_H

namespace must
{
    /**
     * Interface to make persistent handle storage objects destructable
     */
    class I_Destructable
    {
    public:
        /**
         * Erases the information.
         *
         * @return true if this erased the last use of this persistent information, this information should not be used
         * by the user.
         */
        virtual bool erase (void) = 0;

        /**
         * Allows to copy this persistent information.
         * I.e. the persistent info can now be copied
         * and each of the copies needs to call erase
         * respectively.
         * @return true if successful, false otherwise.
         */
        virtual bool copy (void) = 0;
    };
}

#endif /* I_DESTRUCTABLE_H */
