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
 * @file OperationInput.h
 * 		@see gti::weaver::OperationInput
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef OPERATIONINPUT_H
#define OPERATIONINPUT_H

#include <string>
#include <vector>

#include "Input.h"
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
			  * An input of for an Analysis that comes
			  * from the result of an operation.
			  */
			class OperationInput : public Input, virtual public Printable
			{
			public:
			  /**
			   * Invalid object Constructor.
			   */
			  OperationInput ( );

			  /**
			   * Proper Constructor.
			   * For operation inputs that use the actual value
			   * returned by the operation, not its length argument
			   * if it is an array returning operation.
			   * @param targetOperation operation to use as input.
			   * @param id of the mapping to use for input.
			   */
			  OperationInput (Operation *targetOperation, int id);

			  /**
			   * Proper Constructor.
			   * For object which want to select whether they
			   * use the actual return value of the operation
			   * or its length argument (only for array returning
			   * operations).
			   * @param targetOperation operation to use as input.
			   * @param id of the mapping to use for input.
			   * @param wantLen true iff length argument is the input.
			   */
			  OperationInput (Operation *targetOperation, int id, bool wantLen);

			  /**
			   * Destructor.
			   */
			  virtual ~OperationInput ( );

			  /**
			   * Sets the pointer to the operation which is
			   * used as input.
			   * @param new_var pointer to input operation.
			   */
			  void setTargetOperation ( Operation * new_var );

			  /**
			   * Returns a pointer to the operation that
			   * is used as input.
			   * @return pointer or NULL for invalid objects.
			   */
			  Operation * getTargetOperation ( );

			  /**
			   * Prints a string that implements this input.
			   * For a call argument this it the argument
			   * name, for an operation input this is the
			   * result variable name (which needs to be
			   * created with a per call unique name).
			   * @return string
			   */
			  virtual std::string getName ( ) const;

			  /**
			   * Returns the data type of this input.
			   * @return string
			   */
			  virtual std::string getType ( ) const;

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			  /**
			   * Returns the name of the DOT node from
			   * which this input comes, either an
			   * operation name or the given call name
			   * followed by a column and the argument
			   * name.
			   * @param callName name of the from which
			   *        this is input.
			   * @return name of DOT node.
			   */
			  virtual std::string getDotInputNodeName (std::string callName);

			  /**
			   * Returns true iff the array length of the
			   * operation is the return value.
			   * @return true iff array length is input.
			   */
			  bool isLenInput (void);

			  /**
			   * Returns true if this input returns the
			   * array length of an array returned by
			   * an operation (which implies that this is
			   * an operation input).
			   * @return true iff array length is input.
			   */
			  virtual bool isOpArrayLen (void) const;

			  /**
			   * If this input uses an operation as input it
			   * returns a pointer to it along with the used
			   * mapping id.
			   * @param pOutOp pointer to storage for operation
			   *        pointer.
			   * @param pOutId pointer to storage for mapping id.
			   * @return true if this input needs an operation, false otherwise.
			   */
			  virtual bool needsOperation (Operation** pOutOp, int *pOutId);

			  /**
			   * Returns true if this input returns an array.
			   * @return true if array.
			   */
			  virtual bool isArrayInput () const;

			  /**
			   * Only valid for array returning inputs.
			   * Returns the name of the length variable for
			   * the array being returned by this input.
			   * @return name of length variable or its
			   *         constant length.
			   */
			  virtual std::string getLenName () const;

			protected:
			  Operation * myTargetOperation; /**< Pointer to the operation used as input. */
			  int myId; /**< Id of the mapping to use as input.*/
			  bool myWantLen; /**< True if the length argument of the operation is the input.*/
			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // OPERATIONINPUT_H
