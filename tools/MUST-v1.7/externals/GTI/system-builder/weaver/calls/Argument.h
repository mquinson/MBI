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
 * @file Argument.h
 * 		@see gti::weaver::Argument
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <string>
#include <vector>

#include "enums.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			/**
			  * An argument of a function call.
			  */
			class Argument : virtual public Printable
			{
			public:

			  /**
			   * Invalid object Constructor.
			   */
			  Argument ( );

			  /**
			   * Proper constructor.
			   * @param name of the argument.
			   * @param type data type of the argument.
			   * @param intent usage intent of original function for this argument.
			   */
			  Argument (
					  std::string name,
					  std::string type,
					  ArgumentIntent intent,
					  std::string typeAfterArg = "");

			  /**
			   * Destructor.
			   */
			  virtual ~Argument ( );

			  /**
			   * Sets the argument name.
			   * @param new_var name.
			   */
			  void setName ( std::string new_var );

			  /**
			   * Returns the argument name
			   * @return name.
			   */
			  std::string getName ( );

			  /**
			   * Sets the argument type.
			   * @param new_var new type.
			   */
			  void setType ( std::string new_var );

			  /**
			   * Returns the type of the argument.
			   * @return type.
			   */
			  std::string getType ( );

			  /**
			   * Returns the type addition that needs to
			   * be appended after the argument name
			   * for declaration.
			   */
			  std::string getTypeAfterArg (void);

			  /**
			   * Sets usage intent of original function.
			   * @param new_var intent to set.
			   */
			  void setIntent ( ArgumentIntent new_var );

			  /**
			   * Returns argument usage intent of original function.
			   * @return intent.
			   */
			  ArgumentIntent getIntent ( );

			  /**
			   * Returns true if this argument is an array.
			   * @return true if array.
			   */
			  virtual bool isArray (void) const;

			  /**
			   * Returns true iff this argument is an array
			   * that uses an operation as length argument.
			   * Usage: to cast the Argument into
			   * ArrayArgument or ArrayArgumentOp.
			   * @return true or false.
			   */
			  virtual bool isArrayWithLengthOp (void) const;

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
			   * Returns the name of the DOT node that represents this
			   * argument.
			   * @param callName call name to which this argument belongs.
			   * @return DOT name.
			   */
			  virtual std::string getArgumentDotName (std::string callName);

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			protected:
			  std::string myName; /**< Argument name. */
			  std::string myType; /**< Argument type. */
			  std::string myTypeAfterArg; /**< Addition to type that needs to be appended after name for declarations, e.g. in "int array[3]" the "[3]" would use this attribute.*/
			  ArgumentIntent myIntent; /**< Argument usage intent of the original function. */
			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // ARGUMENT_H
