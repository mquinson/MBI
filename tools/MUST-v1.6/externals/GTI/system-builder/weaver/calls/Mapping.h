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
 * @file Mapping.h
 * 		@see gti::weaver::Mapping
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef MAPPING_H
#define MAPPING_H

#include <string>
#include <vector>

#include "Call.h"
#include "Input.h"

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			/**
			  * Mapps how an analysis or operation is applied for
			  * a call, i.e. what call arguments are used as
			  * what analysis/operation inputs.
			  */
			class Mapping : virtual public Printable
			{
			public:
			  /**
			   * Invalid object constructor.
			   */
			  Mapping ( );

			  /**
			   * Proper constructor.
			   * @param apiCall Call to which the operation/analysis is mapped.
			   * @param order determines whether this mapping needs to be
			   *              executed before or after the actual call.
			   * @param id id of this mapping, used to distinguish multiple mappings
			   *           of the same calculation to a call.
			   * @param intraCallOrder order id used to enforce an ordering of multiple analyses/operations
			   *              mapped to one call.
			   */
			  Mapping (Call* apiCall, CalculationOrder order, int id, int intraCallOrder);

			  /**
			   * Destructor
			   */
			  ~Mapping ( );

			  /**
			   * Sets the API call to which the analysis/operation
			   * is applied.
			   * @param new_var API call.
			   */
			  void setApiCall ( Call * new_var );

			  /**
			   * Returns the call to which the analysis/operation
			   * is applied.
			   * @return API call.
			   */
			  Call* getApiCall (void);

			  /**
			   * Adds a input for the analysis/operation, the
			   * order in which the inputs are added is associated
			   * with the arguments described in the input description
			   * of the analysis/argument.
			   * @param add_object input to add.
			   */
			  void addArgumentInput ( Input * add_object );

			  /**
			   * Returns the list of inputs for this mapping.
			   * @return list of inputs.
			   */
			  std::vector<Input*> getArgumentInputs (void);

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			  /**
			   * Returns the order of this calculation.
			   * @see CalculationOrder
			   * @return order.
			   */
			  CalculationOrder getOrder (void);

			  /**
			   * Sets the order of this calculation.
			   * @see CalculationOrder
			   * @param order new order.
			   */
			  void setOrder (CalculationOrder order);

			  /**
			   * Returns the mapping id which is used to
			   * distinguish multiple mappings of one
			   * analysis to one call.
			   * @return id of mapping, or -1 if no id
			   *         was given for this mapping.
			   */
			  int getId (void);

			  /**
			   * Returns the intra call order of this mapping.
			   * @see gti::weaver::calls::Mapping.myIntraCallOrder.
			   * @return intra call order.
			   */
			  int getIntraCallOrder (void);

			protected:
			  Call * myApiCall; /**< Call to which the operation/analysis is mapped. */
  			  std::vector<Input*> myArgumentInputs; /**< List of inputs, order reflects the order in the argument/operation input specification. */
  			  CalculationOrder myOrder;
  			  int myId; /**< Mapping id, used to refer to a distinct mapping of an operations as it may be mapped multiple times to some function.*/

  			  /**
  			   * Used to enforce an order for multiple analyses/operations mapped to one call
  			   * (If the mapping is for an anylsis, the order only applies to all analyses mapped to the call;
  			   * If the mapping is for an operation, the order only applies to all operations mapped to the call;
  			   * i.e. the order enforces no ordering between analyses and operations).
  			   *
  			   * Lower order means that the op/analysis must be executed before all other ops/analyses with
  			   * a higher order.
  			   *
  			   * Values of 0 and > 0 are used for user specified values.
  			   * Negative values are reserved for internal operations and analyses.
  			   */
  			  int myIntraCallOrder;
			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace must*/
#endif // MAPPING_H
