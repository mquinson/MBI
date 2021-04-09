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
 * @file CallProperties.h
 * 		@see gti::weaver::CallProperties
 *
 * @author Tobias Hilbrich
 * @date 05.08.2010
 */

#ifndef CallProperties_H
#define CallProperties_H

#include <string>
#include <list>

#include "Input.h"
#include "Call.h"
#include "Operation.h"

using namespace gti::weaver::calls;

namespace gti
{
	namespace weaver
	{
		namespace layout
		{
			/**
			 * Stores properties of a level for a call.
			 * Used to store important information
			 * for the generation step, e.g. whether a
			 * call needs to be wrapped.
			 * It used:
			 *  - a list of used arguments:
			 *    Arguments that are used by calculations
			 *    on this level.
			 *  - a list of arguments to receive:
			 *    The used arguments + arguments needed
			 *    by descendants of this level.
			 *  - a unique ID for the record that brings
			 *    the arguments to receive to the level.
			 *  - list of operations that need to be
			 *    executed to calculate some of the inputs
			 *    for this call.
			 */
			class CallProperties : virtual public Printable
			{
			public:
				/**
				 * Invalid object Constructor.
				 */
				CallProperties (void);

				/**
				 * Proper constructor
				 * @param call for which these properties are
				 * @param isOnApplication flag that specifies whether the properties
				 *        are for the application level or not.
				 */
				CallProperties (Call* call, bool isOnApplication = false);

				/**
				 * Destructor
				 */
				~CallProperties (void);

				/**
				 * Adds a list of arguments to the used argument
				 * list.
				 * @param args to add.
				 */
				void addUsedArgs (std::list<Input*> args);
				/**
				 * Adds a list of arguments to the list of
				 * arguments to receive.
				 * @param args to add.
				 */
				void addArgsToReceive (std::list<Input*> args);

				/**
				 * Returns the list of used arguments.
				 * @return list of used arguments.
				 */
				std::list<Input*> getUsedArgs (void);

				/**
				 * Returns the list of arguments to receive.
				 * @return list of arguments to receive.
				 */
				std::list<Input*> getArgsToReceive (void);

				/**
				 * Compares the given list of inputs to receive
				 * with this properties list of inputs to receive.
				 * @return true if both lists have the same inputs,
				 *         false otherwise.
				 */
				bool receiveListEqual (std::list<Input*> other);

				/**
				 * Returns true if the usedArgs or the argsToReceive of this property
				 * contain all of the given inputs.
				 * @return true if args contain all of the given inputs.
				 */
				bool usedOrReceiveArgsContain (std::list<Input*> other);

				/**
				 * Returns the unique id of the record
				 * that brings the arguments to receive
				 * to this level.
				 * @return unique id if set, -1 otherwise.
				 */
				long long getInRecordUniqueId (void);
				/**
				 * Sets the unique id of the record that
				 * brings the arguments to receive to
				 * this level.
				 * @param uid the unique is to set.
				 */
				void setInRecordUniqueId (long long uid);

				/**
				 * Returns true if the level needs a
				 * wrapper for this call.
				 * @return true or false.
				 */
				bool needsWrapper (void) const;

				/**
				 * Returns true if the level needs a
				 * receival module for this call,
				 * note that the actual decission also
				 * depends on the fact whether this is
				 * the application level (which never
				 * needs receival modules) or not.
				 * @return true or false.
				 */
				bool needsReceival (void) const;

				/**
				 * Adds an operation to the list of operations to
				 * execute prior to creating records for this call
				 * or calling its analyses.
				 */
				void addOperationToExecute (Operation *op, int id);

				/**
				 * Returns the operations executed for this call
				 * along with their mapping ids.
				 * @return list of ops and mapping ids.
				 */
				std::list<std::pair<Operation*, int > > getMappedOperations (void);

				/**
				 * Static method that returns the next
				 * unused unique id, which is used to
				 * identify records.
				 */
				static long long getNextUniqueId (void);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

				/**
				 * Notifies the property that it is for a wrapp across call that is actually
				 * being called on this level, i.e. a wrapper for it is needed if there
				 * is any required information.
				 */
				void setWrapAcrossCallAsCreatedOnLevel (void);

			protected:
				Call* myCall; /**< call for which these properties are.*/
				bool myInformationRequired; /**< True if information for this call (possibly not any data but just its occurance is required on this level).*/

				std::list<Input*> myUsedArgs; /**< Arguments used on this level for this call.*/
				std::list<Input*> myArgsToReceive; /**< Arguments that need to be received on this level for this call.*/
				long long myInRecordUniqueId; /**< The unique id of the records that bring the arguments to this level.*/
				bool myWrapAcrossIsCreatedOnLevel; /**< True if this is a property for a wrap across call that is created on this level.*/

				std::list<std::pair<Operation*, int > > myOpsToExecute; /**< List of operations and their mapping id to this call, which are executed before creating records or calling analyses.*/

				bool myIsOnApplication; /**< True if these properties are for the application level.*/

				static long long ourNextUniqueId; /**< Static attribute for the next free unique id.*/

				/**
				 * Helper to add args to one of the two lists.
				 * @param args the args to add.
				 * @param target pointer to the list to which to add.
				 */
				void addArgs (std::list<Input*> args, std::list<Input*> *target);
			};
		} /*namespace layout*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // CallProperties_H
