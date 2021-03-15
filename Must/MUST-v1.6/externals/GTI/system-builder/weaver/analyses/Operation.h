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
 * @file Operation.h
 * 		@see gti::weaver::Operation
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef OPERATION_H
#define OPERATION_H

#include <string>

#include "Printable.h"
#include "Calculation.h"

namespace gti
{
	namespace weaver
	{
		namespace analyses
		{
			/**
			  * class Operation
			  *
			  */
			class Operation : public Calculation, virtual public Printable
			{
			public:
				friend std::ostream& operator<<(std::ostream& out, const Operation& l);

			  /**
			   * Invalid object Constructor.
			   */
			  Operation ( );

			  /**
			   * Proper Constructor.
			   * @param name operation name.
			   * @param argumentSpec input specification of the operation.
			   * @param group analysis group to which the operation belongs.
			   * @param returnType datatype returned by the operation.
			   * @param extraHeaders headers needed for this operation.
			   * @param sourceTemplate operation source with templates.
			   * @param cleanupTemplate operation source with templates for cleanup of resources.
			   */
			  Operation (
					  std::string name,
					  std::vector<InputDescription*> argumentSpec,
					  AnalysisGroup * group,
					  std::string returnType,
					  std::list<std::string> extraHeaders,
					  std::string sourceTemplate,
					  std::string cleanupTemplate);

			  /**
			   * Proper Constructor for operations that return an
			   * array.
			   * @param name operation name.
			   * @param argumentSpec input specification of the operation.
			   * @param group analysis group to which the operation belongs.
			   * @param returnType datatype returned by the operation.
			   * @param extraHeaders headers needed for this operation.
			   * @param sourceTemplate operation source with templates.
			   * @param cleanupTemplate operation source with templates for cleanup of resources.
			   * @param arrayLenType type of the array length.
			   */
			  Operation (
					  std::string name,
					  std::vector<InputDescription*> argumentSpec,
					  AnalysisGroup * group,
					  std::string returnType,
					  std::list<std::string> extraHeaders,
					  std::string sourceTemplate,
					  std::string cleanupTemplate,
					  std::string arrayLenType);

			  /**
			   * Destructor.
			   */
			  virtual ~Operation ( );

			  /**
			   * Sets the return type for this operation.
			   * @param new_var new type.
			   */
			  void setReturnType ( std::string new_var );

			  /**
			   * Returns the datatype returned by this operation.
			   * @return datatype.
			   */
			  std::string getReturnType ( );

			  /**
			   * Returns the data type used for the length argument
			   * of an operation that returns an array.
			   * @return datatype.
			   * @see hasArrayReturn
			   */
			  std::string getLenReturnType ( );

			  /**
			   * Sets the list of extra headers needed for this
			   * operation.
			   * @param new_var list of headers.
			   */
			  void setExtraHeaders ( std::list<std::string> new_var );

			  /**
			   * Returns the list of additional headers needed
			   * for this operation.
			   * @return list of headers.
			   */
			  std::list<std::string> getExtraHeaders ( );

			  /**
			   * Sets the source template for this operation.
			   * @param new_var template.
			   */
			  void setSourceTemplate ( std::string new_var );

			  /**
			   * Returns the source template for this operation.
			   * @return source template.
			   */
			  std::string getSourceTemplate (void);

			  /**
			   * Returns the source template used to clear and
			   * free resources requests in the actual source
			   * template of this operation.
			   * If no cleanup template is needed for this
			   * operation it returns an empty string.
			   * @return cleanup template.
			   */
			  std::string getCleanupTemplate (void);

			  /**
			   * Returns true if this is an operation
			   * and false if it is an analysis.
			   */
			  virtual bool isOperation (void);

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  std::ostream& print (std::ostream& out) const;

			  /**
			   * Returns a string that conatins the name of
			   * the result variable for an instanziation of
			   * this operation.
			   * @param mappingId id of the mapping of the
			   *        operation to a call, important
			   *        to distinguish multiple mappings of
			   *        one operation to one call.
			   * @return variable name.
			   */
			  std::string getResultVarName (int mappingId);

			  /**
			   * Returns a string that conatins the name of
			   * the result variable which holds the length
			   * of the array returned by this operation
			   * for an instanziation of this operation.
			   * Only valid vor array returning operations.
			   * @param mappingId id of the mapping of the
			   *        operation to a call, important
			   *        to distinguish multiple mappings of
			   *        one operation to one call.
			   * @return variable name.
			   * @see hasArrayReturn
			   */
			  std::string getResultLenVarName (int mappingId);

			  /**
			   * Returns the name of the DOT node for this
			   * operation.
			   * @param mappingId id of the mapping.
			   * @return name of node.
			   */
			  virtual std::string getDotNodeName (int mappingId);

			  /**
			   * Returns true if this operations returns an
			   * array value and false otherwise.
			   * @return true if operation returns array, false otherwise.
			   */
			  bool hasArrayReturn (void);

			  /**
			   * Returns a color that should be used to fill the dot node
			   * for this calculation. (The color should represent the
			   * calculation type as well as key properties of it.)
			   * @return DOT color name.
			   */
			  virtual std::string getDotNodeColor (void);

			  /**
			   * Replaces the source template according to the
			   * given mapping and returns the replaced code.
			   * @param m mapping to use.
			   * @return replaced template.
			   */
			  std::string replaceSourceForMapping (Mapping *m);

			  /**
			   * Replaces the cleanup template according to the
			   * given mapping and returns the replaced code.
			   * @param m mapping to use.
			   * @return replaced template.
			   */
			  std::string replaceCleanupForMapping (Mapping *m);

			protected:
			  std::string myReturnType; /**< Data type that is returned by the operation. */
			  std::list<std::string> myExtraHeaders; /**< Headers needed for the operation. */
			  std::string mySourceTemplate; /**< Source with templates for this operation. */
			  std::string myCleanupTemplate; /**< Source with templates to cleanup resources used in the actual source template. */
			  bool myHasArrayReturn; /**< True if the operation returns an array. */
			  std::string myArrayLenType; /**< Type of the array length value of this operation, only of interest for array returning operations. */

			  /**
			   * Replaces placeholders in a code template with the values
			   * of the given mapping.
			   * @param codeTemplate the template to replace.
			   * @param m mapping to replace with.
			   * @return replaced template.
			   */
			  std::string replaceTemplate (std::string codeTemplate, Mapping *m);
			};
		} /*namespace analyses*/
	} /*namespace weaver*/
}/*namespace gti*/
#endif // OPERATION_H
