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
 * @file CallIdInput.h
 * 		@see gti::weaver::CallIdInput
 *
 * @author Tobias Hilbrich
 * @date 07.01.2011
 */

#ifndef CALLIDINPUT_H
#define CALLIDINPUT_H

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
			  * is a unique id that identifies an API call.
			  * (Unique among all specified API calls)
			  */
			class CallIdInput : public Input, virtual public Printable
			{
			public:
			  /**
			   * Invalid object Constructor.
			   */
			  CallIdInput ( );

			  /**
			   * Proper Constructor.
			   */
			  CallIdInput (Call* call);

			  /**
			   * Empty Destructor
			   */
			  virtual ~CallIdInput ( );

			  /**
			   * Returns the argument used as input.
			   * @return argument.
			   */
			  int getCallId (void) const;

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

			protected:
			  Call *myTargetCall; /**< The call whose name is used as input. */
			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // CALLIDINPUT_H
