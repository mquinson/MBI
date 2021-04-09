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
 * @file RequiresApi.h
 * 		@see gti::weaver::RequiresApi
 *
 * @author Tobias Hilbrich
 * @date 14.01.2010
 */

#ifndef REQUIRESAPI_H
#define REQUIRESAPI_H

#include <string>
#include <list>

#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace modules
		{
			/**
			 * Captures a list or required APIs.
			 */
			class RequiresApi : virtual public Printable
			{
			public:
				/**
				 * Constructor
				 */
				RequiresApi ();

				/**
				 * Constructor with arguments.
				 * @param apis list of API names required for this module.
				 */
				RequiresApi (std::list<std::string> apis);

				/**
				 * Destructor
				 */
				virtual ~RequiresApi ( );

				/**
				 * Adds an API to the list of required APIs.
				 */
				void addApi (std::string api);

				/**
				 * Removes an API from the list of required APIs.
				 */
				void removeApi (std::string api);

				/**
				 * Returns the list of required APIs.
				 */
				std::list<std::string> getRequiredApis (void);

				/**
				 * Checks whether API is in the list of required APIs.
				 */
				bool checkRequiredApi (std::string api) const;

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

			protected:
				std::list<std::string> myApis;

			}; /*RequiresApi*/
		} /*namespace modules*/
	} /*namespace weaver*/
} /*namepsace gti*/

#endif // REQUIRESAPI_H
