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
 * @file InputDescription.h
 * 		@see gti::weaver::InputDescription
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef INPUTDESCRIPTION_H
#define INPUTDESCRIPTION_H

#include <string>

#include "Printable.h"

namespace gti
{

	namespace weaver
	{
		namespace analyses
		{
			/**
			  * Used to describe a single input argument of an
			  * Operation or Analysis (->Calculation).
			  */
			class InputDescription : virtual public Printable
			{
			public:
				/**
				 * Invalid Object Constructor.
				 */
				InputDescription ( );

				/**
				 * Invalid Object Constructor.
				 */
				InputDescription (std::string argumentType, std::string name);

				/**
				 * Destructor.
				 */
				virtual ~InputDescription ( );

				/**
				 * Sets the type for the argument.
				 * @param new_var type.
				 */
				void setArgumentType ( std::string new_var );

				/**
				 * Returns the type of the argument.
				 * @return type.
				 */
				std::string getArgumentType ( );

				/**
				 * Returns the name of the described argument.
				 * @return name.
				 */
				std::string getName (void);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

			protected:
				std::string myArgumentType;
				std::string myName;
			};
		}/*namespace analyses*/
	} /*namespace weaver*/
}/*namespace gti*/

#endif // INPUTDESCRIPTION_H
