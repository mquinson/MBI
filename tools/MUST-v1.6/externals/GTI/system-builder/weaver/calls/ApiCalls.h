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
 * @file ApiCalls.h
 * 		@see gti::weaver::ApiCalls
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef APICALLS_H
#define APICALLS_H

#include <string>
#include <vector>
#include <map>

#include "GtiEnums.h"
#include "ApiGroup.h"
#include "SpecificationNode.h"
#include "Input.h"

namespace gti
{
	namespace weaver
	{
		namespace calls
		{
			/**
			  * Singleton to manage all APIs.
			  */
			class ApiCalls
			{
			public:
			  /**
			   * Add an API to the myApis List
			   */
			  void addApi ( ApiGroup * add_object );

			  /**
			   * Remove an API from myApis List
			   */
			  void removeApi ( ApiGroup * remove_object );

			  /**
			   * Returns the singleton instance.
			   * @return instance.
			   */
			  static ApiCalls* getInstance (void);

			  /**
			   * Loads APIs from the list of specifications.
			   * @param apiSpecificationXmls list of APIs.
			   */
			  GTI_RETURN load (std::list<std::string> apiSpecificationXmls );

			  /**
			   * Writes nodes for all functions and their arguments to dot files.
			   * The file names are built by the baseFileName and the name
			   * of the respective API Specification name.
			   * @param baseFileName name preposition for output files.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN writeFunctionsAsDot (std::string baseFileName);

			  /**
			   * Returns a list of all headers used for APIs.
			   * @return list of headers.
			   */
			  std::list<std::string> getAllApiHeaders (void);

			  /**
			   * Returns a list of all calls that are used as finalizers.
			   * The calls in the list must not be freed, their memory
			   * is handled by other objects.
			   * @return list of finalizer calls.
			   *
			   * @see gti::weaver::calls::Call::isFinalizer
			   */
			  std::list<Call*> getFinalizerCalls (void);

			protected:

			  /**
			   * Constructor.
			   */
			  ApiCalls ( );

			  /**
			   * Destructor.
			   */
			  virtual ~ApiCalls ( );

			  /**
			   * Reads a specification XML starting at the root node.
			   * @param root the node to start reading (root).
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readApiSpecification (SpecificationNode root);

			  /*
			   * Reads a function description.
			   * @param node the function node.
			   * @param group the API group to add the function to.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readFunction (SpecificationNode node, ApiGroup *group);

			  /**
			   * Reads a function-argument node.
			   * @param node to read.
			   * @param ppOutArgument pointer to an Argument pointer, used to store a
			   *        a pointer to the new Argument in.
			   * @param pOutOrder pointer to int value, used to store the order of
			   *        the Argument in.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readArgument (SpecificationNode node, Argument **ppOutArgument, int *pOutOrder);

			  /**
			   * Reads a function-argument node.
			   * @param node to read.
			   * @param ppOutArgument pointer to an Argument pointer, used to store a
			   *        a pointer to the new Argument in.
			   * @param pOutOrder pointer to int value, used to store the order of
			   *        the Argument in.
			   * @param argMap map (order,Argument) for all non array arguments of current call,
			   *        needed to determine length arguments.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readArrayArgument (SpecificationNode node, Argument **ppOutArgument, int *pOutOrder, std::map<int, Argument*> argMap);

			  /**
			   * Helper to read all the attributes of a function-argument
			   * or function-array-argument node.
			   * @param node the node.
			   * @param nodeName (either function-argument or function-array-argument).
			   * @param pOutName storage for the read name attribute.
			   * @param pOutType storage for the read type attribute.
			   * @param pOutIntent storage for read attribute intent attribute.
			   * @param pOutOrder storage for read order attribute.
			   * @param pOutTypeAfterArg storage for extra type to append after argument name.
			   * @return true if successful, false otherwise.
			   */
			  bool readArgumentAttributes (
					  SpecificationNode node,
					  std::string nodeName,
					  std::string *pOutName,
					  std::string *pOutType,
					  ArgumentIntent *pOutIntent,
					  int *pOutOrder,
					  std::string *pOutTypeAfterArg);

			  /**
			   * Lists available analysis functions of a group.
			   * @param group name of the group whose functions are to be listed.
			   * @param out stream to write the list to.
			   */
			  void listAvailableAnalysisFunctions(std::string group, std::ostream &out);

			  /**
			   * Reads an analysis node of a call and creates the
			   * respective mapping.
			   * @param node analysis node.
			   * @param call call of the analysis node.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readAnalysis (SpecificationNode node, Call *call, int analysisIndex);

			  /**
			   * Reads an input argument for an analysis.
			   * @param node analysis-argument node.
			   * @param call the call to which the analysis is mapped.
			   * @param pOutOrder order of the calculation argument for which this is the input.
			   * @param ppInput pointer to allocate storage for the read input in.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readAnalysisInputArgument (
					  SpecificationNode node,
					  Call *call,
					  int *pOutOrder,
					  Input **ppInput);

			  /**
			   * Reads an operation node from the specification.
			   * @param node XML node of the operation.
			   * @param call the call to which the operation node belongs.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readOperation (SpecificationNode node, Call* call, int operationIndex);

			  /**
			   * Reads the common attributes of mappings to analyses and
			   * operations.
			   * @param node the analysis/operation node.
			   * @param nodeName either "analysis" or "operation".
			   * @param pGroup storage for group name of calculation.
			   * @param pName storage for name of calculation.
			   * @param pOrder storage for order of calculation execution.
			   * @return true if successful, false otherwise.
			   */
			  bool readAnalysisOperationAttributes (
					  SpecificationNode node,
					  std::string nodeName,
					  std::string *pGroup,
					  std::string *pName,
					  CalculationOrder *pOrder);

			  /**
			   * Reads a mapped argument starting from the parent of
			   * the "call-arg-name" node that specifies the argument
			   * name.
			   * @param parent parent node of the "call-arg-name" child.
			   * @param parentName name of the parent node.
			   * @param call call to which this input argument belongs.
			   * @return pointer to the newly created input argument if successful,
			   *         NULL otherwise.
			   */
			  Input* readInputOfArgumentType (
					  SpecificationNode parent,
					  std::string parentName,
					  Call* call);

			  std::vector<ApiGroup*> myApis; /**< List of API groups. */
			  static ApiCalls* ourInstance; /**< Static singleton instance. */
			};
		} /*namespace calls*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // APICALLS_H
