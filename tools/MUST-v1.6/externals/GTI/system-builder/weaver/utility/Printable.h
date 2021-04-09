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
 * @file Printable.h
 * 		@see gti::weaver::Printable
 *
 * @author Tobias Hilbrich
 * @date 13.07.2010
 */

#ifndef PRINTABLE_H
#define PRINTABLE_H

#include <iostream>

namespace gti
{
	namespace weaver
	{
		/**
		 * Common base class for all objects that support
		 * output with ostream operators.
		 */
		class Printable
		{
		public:
			/**
			 * Hook method for printing,
			 * each printable object implements this function
			 * in order to enable the "<<" operator.
			 * @param out ostream to use.
			 * @return ostream after printing.
			 */
			virtual std::ostream& print (std::ostream& out) const = 0;
		};

		/**
		 * Prints the object to the given ostream.
		 * @param out ostream to use.
		 * @param p object to print, which implements a print function.
		 * @return ostream after printing.
		 */
		std::ostream& operator << (std::ostream& out, const Printable& p);
	} /*namespace weaver*/
} /*namespace gti*/

#endif /*#ifndef PRINTABLE_H*/
