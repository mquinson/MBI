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
 * @file ApiGroup.h
 * 		@see gti::weaver::ApiGroup
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef APIGROUP_H
#define APIGROUP_H

#include <string>
#include <vector>
#include <list>

#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			class Call; /*forward declaration*/

			/**
			  * A group of API calls.
			  */
			class ApiGroup : virtual public Printable
			{
			public:
				/**
				 * Invalid object constructor.
				 */
				ApiGroup ( );

				/**
				 * Proper constructor.
				 */
				ApiGroup (std::string name);

				/**
				 * Destructor.
				 */
				~ApiGroup ( );

				/**
				 * Adds a header that is required for the
				 * API group.
				 * @param new_var header to add.
				 */
				void addHeader ( std::string new_var );

				/**
				 * Returns list of headers required for this
				 * API group.
				 * @return the value of myHeaders
				 */
				std::list<std::string> getHeaders (void);

				/**
				 * Sets the name of the API group.
				 * @param new_var new name.
				 */
				void setApiName ( std::string new_var );

				/**
				 * Returns the name of the API group.
				 * @return name.
				 */
				std::string getApiName ( );

				/**
				 * Registers the given call as a member of
				 * this API group.
				 * No operation is done if the call was
				 * already registerd.
				 * @param new_call call to register.
				 */
				void registerCall (Call* new_call);

				/**
				 * Returns the list of calls in this group.
				 * @return list of calls.
				 */
				std::list<Call*> getCalls (void);

				/**
				 * Removes given call from the list of
				 * calls in this group. The call will only
				 * be removed, not deleted.
				 * @param call to remove.
				 */
				void removeCall (Call* call);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

			protected:
			  std::list<std::string> myHeaders; /**< List of headers required for this API group. */
			  std::string myApiName; /**< Name of this API group. */
			  std::list<Call*> myCalls; /**< List of calls in this API group. */
			};
		} /*namespace calls*/
	}/*namespace weaver*/
}/*namespace must*/

#include <Call.h>

#endif // APIGROUP_H
