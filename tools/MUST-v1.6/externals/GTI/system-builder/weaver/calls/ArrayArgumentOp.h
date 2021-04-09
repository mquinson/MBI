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
 * @file ArrayArgumentOp.h
 * 		@see gti::weaver::ArrayArgumentOp
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef ARRAYARGUMENTOP_H
#define ARRAYARGUMENTOP_H

#include <string>
#include <vector>

#include "Argument.h"
#include "Operation.h"
#include "Printable.h"

using namespace gti::weaver::analyses;

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			/**
			  * An array argument that uses an operation to
			  * determine its array length.
			  */
			class ArrayArgumentOp : public Argument, virtual public Printable
			{
			public:
			  /**
			   * Invalid object constructor
			   */
			  ArrayArgumentOp ( );

			  /**
			   * Proper Constructor
			   * @param name of the argument.
			   * @param type datatype of the argument.
			   * @param intent usage intent of the datatype.
			   * @param lengthOp the operation that returns the length of this arguments array.
			   * @param id id of the mapping of the operation to this call, used to distinguish multiple mappings of the same operation to one call.
			   */
			  ArrayArgumentOp (
					  std::string name,
					  std::string type,
					  ArgumentIntent intent,
					  Operation* lengthOp,
					  int id,
					  std::string typeAfterArg = "");

			  /**
			   * Proper Constructor
			   * @param name of the argument.
			   * @param type datatype of the argument.
			   * @param intent usage intent of the datatype.
			   * @param lengthOp the operation that returns the length of this arguments array.
			   * @param id id of the mapping of the operation to this call, used to distinguish multiple mappings of the same operation to one call.
			   * @param wantOpLen true iff the operation returns an array and the length argument is the length of that array.
			   */
			  ArrayArgumentOp (
					  std::string name,
					  std::string type,
					  ArgumentIntent intent,
					  Operation* lengthOp,
					  int id,
					  bool wantOpLen,
					  std::string typeAfterArg = "");

			  /**
			   * Destructor
			   */
			  virtual ~ArrayArgumentOp ( );

			   /**
			   * Sets the operation used to determine array length.
			   * @param new_var operation for array length.
			   */
			  void setLengthOp ( Operation * new_var );

			  /**
			   * Returns the operation used to determine array length.
			   * @return operation.
			   */
			  Operation * getLengthOp (void);

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

			  /**
			   * Returns true iff this argument is an array
			   * that uses an operation as length argument.
			   * Usage: to cast the Argument into
			   * ArrayArgument or ArrayArgumentOp.
			   * @return true or false.
			   */
			  virtual bool isArrayWithLengthOp (void) const;

			  /**
			   * Returns the mapping ID used to identify the instance
			   * of the operation.
			   * @return mapping id.
			   */
			  int getMappingId (void);

			protected:
			  Operation * myLengthOp; /**< Operation used to determine array length. */
			  int myId; /**< Mapping id to distinguish multiple mappings one an operation to the same call.*/
			  bool myWantOpLen; /**< True if the length operation is a array returning operation and the length argument is its array length. */
			};
		} /*namespace calls*/
	} /*namesapce weaver*/
} /*namespace gti*/
#endif // ARRAYARGUMENTOP_H
