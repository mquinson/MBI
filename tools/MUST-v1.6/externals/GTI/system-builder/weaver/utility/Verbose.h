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
 * @file Verbose.h
 * 		@see gti::Verbose
 *
 * @author Tobias Hilbrich
 * @date 05.07.2010
 */

#ifndef VERBOSE_H
#define VERBOSE_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "GtiEnums.h"

#define VERBOSE(x) Verbose::getInstance()->getStream(x)

namespace gti
{
	namespace weaver
	{
		/**
		 * Singleton that is used to handle Verbose output.
		 * Levels are:
		 * - 0 no verbose output
		 * - 1 basic output
		 *     (must not contain any ever repeating output!)
		 * - 2 detailed output
		 *     (may flood the user with output!)
		 * - 3 Extremely fine grained output
		 */
		class Verbose
		{
		public:
			/**
			 * Returns the instance of the singleton.
			 * @return instance.
			 */
			static Verbose* getInstance (void);

			/**
			 * Returns a stream to write a message of
			 * the given verbosity to. If the given
			 * verbosity is higher than the verbosity
			 * selected by the user, it may be an output
			 * stream to /dev/null.
			 * @param verbosity verbosity level of the message
			 *                  intented for the returned stream.
			 * @return an output stream.
			 */
			std::ostream& getStream (int verbosity);

			/**
			 * Returns the verbosity level.
			 * @return level.
			 */
			int getLevel (void);

		protected:
			/**
			 * Constructor.
			 */
			Verbose (void);

			/**
			 * Destructor.
			 */
			~Verbose ();

			static Verbose* ourInstance; /**< The instance of this singleton. */
			std::ostream *myStream; /**< Stream for actual output. */
			std::ofstream *myNull; /**< /dev/null stream. */
			int myVerbosity; /**< User selected verbosity level. */
		};/*class Verbose*/
	} /*namespace weaver*/
} /*namespace gti*/

#endif /*VERBOSE_H*/
