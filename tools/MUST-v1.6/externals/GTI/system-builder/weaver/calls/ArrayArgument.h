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
 * @file ArrayArgument.h
 * 		@see gti::weaver::ArrayArgument
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef ARRAYARGUMENT_H
#define ARRAYARGUMENT_H

#include <string>
#include <vector>

#include "Argument.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			/**
			  * An array argument of a call.
			  */
			class ArrayArgument : public Argument, virtual public Printable
			{
			public:
			  /**
			   * Invalid object Constructor
			   */
			  ArrayArgument ( );

			  /**
			   * Proper Constructor
			   */
			  ArrayArgument (
					  std::string name,
					  std::string type,
					  ArgumentIntent intent,
					  Argument* lengthArgument,
					  std::string typeAfterArg = "");

			  /**
			   * Destructor
			   */
			  virtual ~ArrayArgument ( );

			  /**
			   * Sets the pointer to the argument used as length
			   * of the array.
			   * @param new_var pointer to length argument.
			   */
			  void setLengthArgument ( Argument * new_var );

			  /**
			   * Returns the argument used to determine array length.
			   * @return Argument pointer.
			   */
			  Argument* getLengthArgument ( );

			  /**
			   * Returns true if this argument is an array.
			   * @return true if array.
			   */
			  virtual bool isArray (void) const;

			  /**
			   * Returns a string that represents a variable
			   * that holds the length of the array.
			   * Only valid for array arguments.
			   * @return name of variable for length.
			   */
			  virtual std::string getLengthVariable (void);

			  /**
			   * Returns the name of the DOT node for the
			   * length argument of an array argument in the
			   * context of the given call name.
			   * Only valid for array arguments.
			   * @param callName name of call of which this is an
			   *        argument.
			   * @return DOT name.
			   * @see isArray
			   */
			  virtual std::string getLengthVariableDotName (std::string callName);

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			protected:
			  Argument* myLengthArgument; /**< Argument used as array length. */
			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // ARRAYARGUMENT_H
