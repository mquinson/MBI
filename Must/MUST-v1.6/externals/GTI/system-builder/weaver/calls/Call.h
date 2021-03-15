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
 * @file Call.h
 * 		@see gti::weaver::Call
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef CALL_H
#define CALL_H

#include <string>
#include <vector>

#include "ApiGroup.h"
#include "Argument.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			class ApiGroup; /*forward declaration*/

			/**
			  * Represents a function call with arguments.
			  */
			class Call : virtual public Printable
			{
			public:
			  /**
			   * Invalid object Constructor
			   */
			  Call ( );

			  /**
			   * Proper constructor.
			   */
			  Call (
					  std::string name,
					  ApiGroup* group,
					  std::string returnType,
					  bool wrappedEverywhere = false,
					  bool isFinalizer = false,
					  bool isLocalFinalizer = false,
					  bool wrappAcross = false,
					  bool wrappDown = false,
					  bool isShutdownNotify = false,
					  bool isOutOfOrder = false,
                                          bool isCallback = false,
					  bool isHook = false);

			  /**
			   * Destructor
			   */
			  ~Call ( );

			  /**
			   * Sets the API group for this call.
			   * @param new_var new group.
			   */
			  void setGroup ( ApiGroup * new_var );

			  /**
			   * Returns the API group of this call.
			   * @return group.
			   */
			  ApiGroup * getGroup ( );


			  /**
			   * Adds an argument to the list of call
			   * arguments.
			   * @param add_object argument to add.
			   */
			  void addArgument ( Argument * add_object );

			  /**
			   * Removes an argument from the argument list.
			   * @param remove_object argument to remove.
			   */
			  void removeArgument ( Argument * remove_object );

			  /**
			   * Returns the list of all call arguments.
			   * @return list of arguments.
			   */
			  std::vector<Argument *> getArguments ( );

			  /**
			   * Set the value of name
			   * @param new_var the new value of name
			   */
			  void setName ( std::string new_var );

			  /**
			   * Get the value of name
			   * @return the value of name
			   */
			  std::string getName ( );

			  /**
			   * Get the uinque id for this call.
			   * @return id.
			   */
			  int getUniqueId (void);

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			  /**
			   * Searches for the argument with given name
			   * and returns a pointer to it.
			   * @param name of the argument.
			   * @return pointer to argument if found, NULL otherwise.
			   */
			  Argument* findArgument (std::string name);

			  /**
			   * Creates a DOT representation of this call.
			   * @return DOT representation.
			   */
			  std::string toDotNode (void);

			  /**
			   * Returns true if this call must be wrapped on the
			   * application processes as well as the tool places.
			   */
			  bool isWrappedEverywhere (void);

			  /**
			   * Returns true if this call is a wrapper that causes
			   * intra layer communication.
			   * @return true or false.
			   */
			  bool isWrappedAcross (void);

			  /**
			   * Returns true if this call is a wrapper that causes
			   * intra layer communication.
			   * @return true or false.
			   */
			  bool isWrappedDown (void);

			  /**
			   * Returns true if this call shuts down the tool.
			   * Special rules apply for such calls:
			   * - They are wrapped on the application,
			   *   independently on whether
			   *   they are used in any analysis
			   * - The have an receival on all levels, independently
			   *   on whether they are used in any analysis
			   * - The wrapper and recival generation will apply
			   *   special generation rules for them to implement
			   *   the shutdown and finalize
			   * - Any wrappers and recival handlers will forward
			   *   this event to all upwards connected places if this
			   *   place actually shuts down
			   * - A place shuts down when it received a finalizer
			   *   event from each place directly -- not recursively --
			   *   connected in the downwards direction.
			   * @return true if the call is a finalizer
			   */
			  bool isFinalizer (void);

			  /**
			   * True if local finalizer.
			   */
			  bool isLocalFinalizer (void);

			  /**
			   * True if this call shuts down any tool layer where it arrives.
			   */
			  bool isNotifyFinalize (void);

			  /**
			   * True if this call must be processed out of order.
			   */
			  bool isOutOfOrder (void);

                          /**
			   * True if this call is a callback.
			   */
			  bool isCallback (void);

			  /**
			   * True if this call is a PnMPI hook.
			   */
			  bool isHook (void);

			  /**
			   * Returns the datatype that is returned by this call.
			   * @return datatype.
			   */
			  std::string getReturnType (void);

			protected:
			  ApiGroup					*myGroup; /**< API group of this call. */
			  std::vector<Argument*> 	myArguments; /**< List of call arguments. */
			  std::string 				myName; /**< Name of call. */
			  std::string 				myReturnType; /**< The data type returned by this call. */
			  bool						myWrappEverywhere; /**< True if this function needs to be wrapped on all application threads and all other tool places. */
			  bool                      myWrappAcross; /**< True if this function is an intra layer communication function to transfer data from one place to another. */
			  bool                      myWrappDown; /**< True if this function is commmunicated towards the application processes, and not towards the root of the TBON.*/
			  bool                      myIsFinalizer; /**< True if this function should start the shut down of the tool. */
			  bool                      myIsLocalFinalizer; /**< True if this function should shut down the module associated with the wrapper. */
			  bool                      myIsShutdownNotify; /**< True if this is the notification that really shuts down the tool.*/
			  bool                      myIsOutOfOrder; /**< True if this event needs to be processed out of order.*/
                          bool                      myIsCallback; /** True if this is a callback */
			  bool                      myIsHook; /**< True if this is an PnMPI hook. */

			  int myUniqueId; /**< Unique identifier for this call, used for call id inputs (see gti::weaver::CallIdInput).*/
			  static int ourNextUniqueId; /**< Next unique identifier to use.*/
			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // CALL_H
