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
 * @file Input.h
 * 		@see gti::weaver::Input
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef INPUT_H
#define INPUT_H

#include <string>

#include "Printable.h"
#include "enums.h"

/*
 * Very ugly prototypes for cyclic header dependencies.
 */
namespace gti
{
	namespace weaver
	{
		namespace analyses
		{
		class Operation;
		}
	}
}

using namespace gti::weaver::analyses;

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			/**
			  * Abstract class that represents a single input value
			  * to an Analysis or Operation (Calculation).
			  * Different implementations differ in the type of
			  * input that they represent (call arguments or
			  * operation results).
			  */
			class Input : virtual public Printable
			{
			public:
			  /**
			   * Proper constructor
			   */
			  Input ( );

			  /**
			   * Destructor
			   */
			  virtual ~Input ( );

			  /**
			   * Prints a string that implements this input.
			   * For a call argument this is the argument
			   * name, for an operation input this is the
			   * result variable name (which needs to be
			   * created with a per call unique name).
			   * @return string
			   */
			  virtual std::string getName ( ) const = 0;

			  /**
			   * Returns the data type of this input.
			   * @return string
			   */
			  virtual std::string getType ( ) const = 0;

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

			  /**
			   * Returns true if this input returns the
			   * array length of an array returned by
			   * an operation (which implies that this is
			   * an operation input).
			   * @return true iff array length is input.
			   */
			  virtual bool isOpArrayLen (void) const;

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

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
			   * Returns the name of the DOT node from
			   * which this input comes, either an
			   * operation name or the given call name
			   * followed by a column and the argument
			   * name.
			   * @param callName name of the from which
			   *        this is input.
			   * @return name of DOT node.
			   */
			  virtual std::string getDotInputNodeName (std::string callName) = 0;

			protected:

			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace gti*/

#include "Operation.h"

#endif // INPUT_H
