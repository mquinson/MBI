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
 * @file Calculation.h
 * 		@see gti::weaver::Calculation
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef CALCULATION_H
#define CALCULATION_H

#include <string>
#include <vector>
#include <list>

#include "Printable.h"
#include "InputDescription.h"
#include "AnalysisGroup.h"
#include "enums.h"

/*
 * Very ugly prototypes for cyclic header dependencies.
 */
namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			class Mapping;
			class Call;
		}
	}
}

using namespace gti::weaver::calls;

namespace gti
{
	namespace weaver
	{
		namespace analyses
		{
			class AnalysisGroup; /**< Forward declaration. */

			/**
			  * class Calculation
			  *
			  */
			class Calculation : virtual public Printable
			{
			public:

			  /**
			   * Empty Constructor.
			   */
			  Calculation ( );

			  /**
			   * Proper Constructor.
			   * @param name of the calculation.
			   * @param argumentSpec specification of expected input.
			   * @param group analysis group of this calculation.
			   */
			  Calculation (
					  std::string name,
					  std::vector<InputDescription*> argumentSpec,
					  AnalysisGroup * group);

			  /**
			   * Empty Destructor.
			   */
			  virtual ~Calculation ( );

			  /**
			   * Adds a mapping for this calculation.
			   * @param add_object mapping to add.
			   */
			  bool addCallMapping ( Mapping * add_object );

			  /**
			   * Returns the list of all mappings for this calculation.
			   * @return list of mappings.
			   */
			  std::vector<Mapping *> getCallMappings ( );

			  /**
			   * Returns the mapping of this calculation to the given
			   * call.
			   * May return a list of mappings if multiple mappings
			   * of one calculation to one call exist.
			   * @param call for which to find a mapping.
			   * @return mapping if found, NULL otherwise.
			   */
			  std::list<Mapping*> getMappingsForCall (Call* call);

			  /**
			   * Returns the mapping of this calculation to the given
			   * call.
			   * @param call for which to find a mapping.
			   * @param id the mapping id to search for.
			   * @return mapping if found, NULL otherwise.
			   */
			  Mapping* getMappingForCall (Call* call, int id);

			  /**
			   * Adds an argument specification for this calculation.
			   * @param add_object argument specification to add.
			   */
			  void addArgumentSpec ( InputDescription * add_object );

			  /**
			   * Returns the full argument specification list.
			   * @return argument specification list.
			   */
			  std::vector<InputDescription *> getArgumentSpec ( );

			  /**
			   * Sets group to which this analysis belongs.
			   * @param new_var pointer to new group.
			   */
			  void setGroup ( AnalysisGroup * new_var );

			  /**
			   * Returns the group to which this calculation belongs.
			   * @return group.
			   */
			  AnalysisGroup * getGroup ( );

			  /**
			   * Sets the name of this calculation.
			   * @param new_var name to set.
			   */
			  void setName ( std::string new_var );

			  /**
			   * Returns the name of this calculation.
			   * @return name.
			   */
			  std::string getName ( );

			  /**
			   * Returns true if this is an operation
			   * and false if it is an analysis.
			   */
			  virtual bool isOperation (void) = 0;

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			  /**
			   * Creates a DOT node for this calculation that
			   * uses the calculation name as label and identifier.
			   * Also prints the input argument types.
			   * @return string with the DOT node description.
			   */
			  std::string toDotNode (void);

			  /**
			   * Like Calculation::toDotNode, but prints the arguments
			   * on top of the calculation name.
			   * @return string with the DOT node description.
			   */
			  std::string toDotNodeBottomUp (void);

			  /**
			   * Returns the name of this analysis function used in DOT files.
			   * (Replaced ":" with a "_")
			   * @return name for DOT.
			   */
			  std::string getDotName (void);

			  /**
			   * Checks whether this calculation has a mapping
			   * to named call in named group.
			   * @param callName name of the call.
			   * @param apiName name of the API group of the call.
			   * @return true if such a mapping exists, false otherwise.
			   */
			  bool hasMappingForCall (std::string callName, std::string apiName);

			  /**
			   * Checks whether this calculation has a mapping
			   * to named call in named group with given id.
			   * @param callName name of the call.
			   * @param apiName name of the API group of the call.
			   * @param id mapping id to check for.
			   * @return true if such a mapping exists, false otherwise.
			   */
			  bool hasMappingForCall (std::string callName, std::string apiName, int id);

			  /**
			   * Checks whether this calculation has a mapping
			   * to named call in named group with given id and order.
			   * @param callName name of the call.
			   * @param apiName name of the API group of the call.
			   * @param id mapping id to check for.
			   * @param order the mapping must have.
			   * @return true if such a mapping exists, false otherwise.
			   */
			  bool hasMappingForCall (std::string callName, std::string apiName, int id, CalculationOrder order);

			  /**
			   * Returns a color that should be used to fill the dot node
			   * for this calculation. (The color should represent the
			   * calculation type as well as key properties of it.)
			   * @return DOT color name.
			   */
			  virtual std::string getDotNodeColor (void) = 0;

			protected:

			  std::string myName; /**< Name of this calculation. */

			  std::vector<Mapping*> myCallMappings; /**< Mapping of this calculation to calls. */

			  std::vector<InputDescription*> myArgumentSpec; /**< Specification of the input expected by this calculation. */

			  AnalysisGroup * myGroup; /**< Group to which this calculation belongs. */
			};
		} /*namespace analyses*/
	} /*namespace weaver*/
} /*namespace gti*/

#include "Operation.h"
#include "Analysis.h"
#include "Mapping.h"

#endif // CALCULATION_H
