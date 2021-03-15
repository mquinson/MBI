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
 * @file Analyses.h
 * 		@see gti::weaver::Analyses
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef ANALYSES_H
#define ANALYSES_H

#include <string>
#include <vector>
#include <list>
#include <map>

#include "GtiEnums.h"
#include "AnalysisGroup.h"
#include "SpecificationNode.h"

namespace gti
{
	namespace weaver
	{
		namespace analyses
		{
			/**
			 * Little helper struct for dependencies.
			 */
			struct Dependency
			{
				std::string groupName;
				std::string moduleName;
				bool isSoft;
			};

			/**
			 * Stores information of all Analyses, is a singleton.
			 */
			class Analyses
			{
			public:
			  /**
			   * Returns the Analyses instance.
			   * @return instance
			   */
			  static Analyses* getInstance (void);

			  /**
			   * Loads a list of analysis specifications and adds their data.
			   * @param  analysisSpecificationXmls list of file names.
			   */
			  GTI_RETURN load (std::list<std::string> analysisSpecificationXmls );

			  /**
			   * Writes all analyses as a dot file for graphical representation.
			   * Uses one output file for each analysis group that was used
			   * to initialize Analyses.
			   * @param fileNameBase used to create the name of the DOT files
			   *        the name is [fileNameBase][AnalysisGroupName].dot.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN writeAnalysesAsDot (std::string fileNameBase);

			  /**
			   * Searches for named operation in the named group.
			   * @param opName name of the operation.
			   * @param groupName name of the analysis group.
			   * @return pointer to Operation if found, NULL otherwise.
			   */
			  Operation* findOperation (std::string opName, std::string groupName);

			  /**
			   * Searches for named analysis in the named group.
			   * Where name is built as [AnalysisModuleName]:[AnalysisFunctionName].
			   * @param name name of the analysis.
			   * @param groupName name of the analysis group.
			   * @return pointer to Analysis if found, NULL otherwise.
			   */
			  Analysis* findAnalysis (std::string name, std::string groupName);

			  /**
			   * Searches for named analysis module in the named group.
			   * @param name name of the analysis module.
			   * @param groupName name of the analysis group.
			   * @return pointer to AnalysisModule if found, NULL otherwise.
			   */
			  AnalysisModule* findAnalysisModule (std::string name, std::string groupName);

			  /**
			   * Returns all known analyses of a group.
			   * Returns all analyses of all groups If the given group name is "".
			   *
			   * @param groupName name of the analysis group or "" to get all analyses.
			   * @return list to pointers to analyses of given group or of all groups.
			   */
			  std::list<Analysis*> getAnalyses(std::string groupName);

			  /**
			   * Returns true if the named group was loaded
			   * as an anylsis group.
			   * @param group name of group to search.
			   * @return true if loaded.
			   */
			  bool hasGroup (std::string group);

			  /**
			   * Adds information on a potential type mismatch.
			   * @param typeA first type of the missmatch.
			   * @param typeB second type of the missmatch.
			   * @param occurrenceDesc textual description of where the match was attempted, e.g. "mapping analysis X to call Y".
			   */
			  void addTypeMissmatchWarning (std::string typeA, std::string typeB, std::string occurrenceDesc);

			  /**
			   * Prints a list of all potential type missmatches and their
			   * first occurance.
			   * @param out stream to write the warnings to.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN printTypeMissmatchWarnings (std::ostream& out);

			  /**
			   * Checks for the correctness of reductions, as they have
			   * several limitations in terms of their mapping.
			   * Must be called after reading all API specifications,
			   * otherwise it will necessarily return an error.
			   * @return GTI_SUCCESS if all reductions are correctly used.
			   *
			   * Even if returning GTI_SUCCESS it may print warnings onto
			   * std::cerr if reductions had to be removed while being valid.
			   * (This happens if multiple analyses are mapped to the same
			   * pre/post event of an API call)
			   */
			  GTI_RETURN checkCorrectnessOfReductions (void);

			  /**
			   * Writs a DOT file for each API call and shows
			   * how its attributes are forwarded to individual
			   * analyses or operations.
			   * @param baseName preposition to use for output file.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN writeMappingsAsDot (std::string baseName);

			  /**
			   * Adds internal analysis groups and operations.
			   * @return GTI_RETURN if successful.
			   */
			  GTI_RETURN addInternalOperations (void);

			protected:
			  /**
			   * Empty Constructor
			   */
			  Analyses ( );

			  /**
			   * Empty Destructor
			   */
			  virtual ~Analyses ( );

			  /**
			   * Add a Group object to the myGroups List
			   */
			  void addGroup ( AnalysisGroup *add_object );

			  /**
			   * Remove a Group object from myGroups List
			   */
			  void removeGroup ( AnalysisGroup * remove_object );

			  /**
			   * Reads an analysis specification starting at the
			   * rood node of the input XML.
			   * @param root node to start reading at.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readAnalysisSpecification (SpecificationNode root);

			  /**
			   * Reads an operation from an analysis specification and
			   * adds it to the given group.
			   * @param node operation node.
			   * @param group to add to.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readOperation (SpecificationNode node, AnalysisGroup *group);

			  /**
			   * Reads an analysis from an analysis specification and
			   * adds it to the given group.
			   * @param node analysis node.
			   * @param group to add to.
			   * @param pOutAnalysisName pointer to string, used to set the string
			   *        to the name of the read analysis.
			   * @param pOutDependencies pointer to a string list which is used
			   *        to store dependencies of this analysis.
			   * @param pOutReductions
			   *  	pointer to a string list which is used
			   *    to store supported reductions of this analysis.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readAnalysis (
					  SpecificationNode node,
					  AnalysisGroup *group,
					  std::string *pOutAnalysisName,
					  std::list<Dependency> *pOutDependencies,
					  std::list<Dependency> *pOutReductions);

			  /**
			   * Reads an dependencies node.
			   * Returns the names of the dependencies in the given list.
			   * @param node dependencies node.
			   * @param pOutDependencies pointer to a string list that is used to hold the dependencies.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readDependencies (SpecificationNode node, std::list<Dependency> *pOutDependencies);

			  /**
			   * Reads an analysis-function node.
			   * @param node the node to read.
			   * @param pOutFunctionName pointer to storage for analysis function name.
			   * @param pOutArgumentSpec pointer to list for all arguments read from the specification.
			   * @param pOutNeedsChannelId pointer to storage for bool, is set to true if the analysis function
			   *              specifies that it needs a channel id as an extra argument.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readAnalysisFunction(
					  SpecificationNode node,
					  std::string *pOutFunctionName,
					  std::vector<InputDescription*> *pOutArgumentSpec,
					  bool *pOutNeedsChannelId);

			  /**
			   * Reads the arguments of an analysis or operation.
			   * @param node operation-arguments or analysis-arguments node.
			   * @param argumentNodeName name of the argument nodes.
			   * @param calculationName name of analysis or operation being handled
			   *        (for error output).
			   * @param pOutArgumentSpec output argument specification.
			   * @return GTI_SUCCESS if successful.
			   */
			  GTI_RETURN readArguments (SpecificationNode node, std::string argumentNodeName, std::string calculationName, std::vector<InputDescription*> *pOutArgumentSpec);

			  /**
			   * Searches for named group and returns its pointer.
			   * @param name of group.
			   * @return pointer to group if found, NULL otherwise.
			   */
			  AnalysisGroup* getGroup (std::string name);

			  //=====================================
			  // Attributes
			  //=====================================
			  std::vector<AnalysisGroup*> myGroups; /**< List of analysis groups. */

			  /**
			   * Stores type mismatches, the alphabetically first type
			   * is ues as first key, the alphabetically lower type of
			   * a missmatch is used as second key, the final string
			   * is used to refer to the occurrance of the missmatch,
			   * e.g. "mapping analysis X to call Y".
			   */
			  std::map<std::string, std::map<std::string, std::string> > myTypeWarnings;

			  static Analyses* myInstance; /**< Singleton instance. */

			};
		} /*namespace analyses*/
	} /*namespace weaver*/
} /*namespace gti*/

#endif // ANALYSES_H
