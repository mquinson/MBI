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
 * @file CallReturnInput.h
 * 		@see gti::weaver::CallReturnInput
 *
 * @author Tobias Hilbrich
 * @date 03.08.2010
 */

#ifndef CALLRETURNINPUT_H
#define CALLRETURNINPUT_H

#include <string>
#include <vector>

#include "Input.h"
#include "Call.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			/**
			  * An input to an operation or analysis which
			  * is the name of an API call.
			  */
			class CallReturnInput : public Input, virtual public Printable
			{
			public:
			  /**
			   * Invalid object Constructor.
			   */
			  CallReturnInput ( );

			  /**
			   * Proper Constructor.
			   */
			  CallReturnInput (Call* call);

			  /**
			   * Empty Destructor
			   */
			  virtual ~CallReturnInput ( );

			  /**
			   * Returns the argument used as input.
			   * @return argument.
			   */
			  std::string getCallName (void);

			  /**
			   * Prints a string that implements this input.
			   * For a call argument this it the argument
			   * name, for an operation input this is the
			   * result variable name (which needs to be
			   * created with a per call unique name).
			   * @return string
			   */
			  std::string getName ( ) const;

			  /**
			   * Returns the data type of this input.
			   * @return string
			   */
			  std::string getType ( ) const;

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
			  Call *myTargetCall; /**< The call whose name is used as input. */
			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // CALLRETURNINPUT_H
