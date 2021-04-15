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
 * @file AnalysisModule.h
 * 		@see gti::weaver::AnalysisModule
 *
 * @author Tobias Hilbrich
 * @date 26.11.2010
 */

#ifndef ANALYSISMODULE_H
#define ANALYSISMODULE_H

#include <string>
#include <vector>
#include <map>

#include "Printable.h"
#include "Analysis.h"
#include "Module.h"
#include "enums.h"
#include "Call.h"

using namespace gti::weaver::modules;

namespace gti
{
	namespace weaver
	{
		namespace analyses
		{
			/*Forward declarations*/
			class Analysis;

			/**
			 * Specification for a module that performs a certain type of analysis.
			 * It may have multiple analysis functions that receive trace data
			 * to perform the analysis, each of these functions is represented by
			 * one Analysis class.
			 */
			class AnalysisModule : public Module, virtual public Printable
			{
			public:
				friend std::ostream& operator<<(std::ostream& out, const AnalysisModule& l);

				/**
				 * Empty Constructor
				 */
				AnalysisModule ( );

				/**
				 * Proper Constructor
				 */
				AnalysisModule (
						std::string name,
						std::string moduleName,
						std::string configModuleName,
						std::string instanceType,
						std::string headerName,
						std::string incDir,
						bool isGlobal,
						bool isProcessGlobal,
						bool isLocalIntegrity = false,
						bool isReduction = false,
						bool listensToTimeouts = false,
						bool isContinuous = false,
						bool isAddedAutomagically = false,
						std::string subGroupName = "General");

				/**
				 * Destructor
				 */
				virtual ~AnalysisModule ( );

				/**
				 * Adds a dependency to the list of dependencies.
				 * @param add_object the dependency to add
				 * @param isSoft true if this is a soft dependency (see isSoftDependency).
				 */
				void addDependency ( AnalysisModule * add_object, bool isSoft = false );

				/**
				 * Removes a dependency from this analysis.
				 * @param remove_object the dependency to remove.
				 */
				void removeDependency ( AnalysisModule * remove_object );

				/**
				 * Returns the list of analyses on which this Analysis
				 * depends.
				 * @return list of dependencies
				 */
				std::list<AnalysisModule*> getDependencies (void);

				/**
				 * Returns true if this dependency is a soft dependency,
				 * i.e. it does not causes the dependent module to
				 * be a child module in the module configuration.
				 * This is needed to break cyclic dependencies that
				 * may appear otherwise.
				 * @param dependency dependent module to check for.
				 * @param true if the dependency to given module is soft, false otherwise.
				 */
				bool isSoftDependency (AnalysisModule* dependency);

				/**
				 * Adds a reduction to the list of reductions supported by this module.
				 * @param add_object the reduction to add
				 */
				bool addSupportedReduction ( AnalysisModule * add_object );

				/**
				 * Empties the list of reductions supported by this module.
				 */
				void removeAllSupportedReductions (void);

				/**
				 * Returns the list of reductions supported by this module.
				 * @return list of reductions
				 */
				std::list<AnalysisModule*> getSupportedReductions (void);

				/**
				 * Returns true if this Analysis is global.
				 * @return true if yes, false otherwise.
				 */
				bool isGlobal (void);

				/**
				 * Returns true if this analysis is global for
				 * all threads of a process.
				 * @return true if yes, false otherwise.
				 */
				bool isProcessGlobal (void);

				/**
				 * Returns true if this is a reduction analysis.
				 * @return true iff reduction.
				 */
				bool isReduction (void);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

				/**
				 * Returns the subGroupName of this analysis.
				 * Usage is purely for presentation purposes,
				 * this name must not have any semantical
				 * consequences.
				 * @return name.
				 */
				std::string getSubGroupName (void) const;

				/**
				 * Returns true if this analysis is a local
				 * integrity analysis (properties see
				 * analysis-specification.dtd). Returns
				 * false otherwise.
				 *
				 * @return true if local integrity.
				 */
				bool isLocalIntegrity (void) const;

				/**
				 * Returns true if analyses of this module are added automagically, i.e.,
				 * when all of their inputs are present anyways.
				 * @return true if module uses automagic mapping.
				 */
				bool isAddedAutomagically (void) const;

				/**
				 * Adds the analysis with the given name and input specification.
				 * @param analysisFunctionName name of the analysis.
				 * @param argumentSpec specification of the arguments of the analysis.
				 * @param group the analysis group of the analysis, all analyses of
				 *              an analysis module must be from the same group.
				 * @param needsChannelId set to true if this analysis function needs
				 *              a channel id as an extra argument.
				 * @return true if successfull, false otherwise.
				 */
				bool addAnalysis (
						std::string analysisFunctionName,
						std::vector<InputDescription*> argumentSpec,
						AnalysisGroup * group,
						bool needsChannelId);

				/**
				 * Returns the analysis with the given name.
				 * @param functionName name of the analysis to find.
				 * @return pointer to the analysis if found, NULL otherwise.
				 *
				 * The memory pointed to by the return value is still managed
				 * by the analysis module and must not be freed by the user.
				 */
				Analysis* findAnalysis (std::string functionName);

				/**
				 * Returns the list of all analyses in this analysis module.
				 * @return list of analyses.
				 *
				 * The memory pointed to by the entries in the returned list is still managed
				 * by the analysis module and must not be freed by the user.
				 */
				std::list <Analysis*> getAnalyses (void);

				/**
				 * Returns the name of this analysis.
				 * @return name.
				 */
				std::string getName (void);

				/**
				  * Returns the group to which this calculation belongs.
				  * @return group.
				  */
				AnalysisGroup * getGroup (void);

				/**
				 * Returns a color that should be used to fill the dot node
				 * for this calculation. (The color should represent the
				 * calculation type as well as key properties of it.)
				 * @return DOT color name.
				 */
				virtual std::string getDotNodeColor (void);

				/**
				 * Returns true if this analysis module has an analysis
				 * with a mapping to the given call and order.
				 * @param call for which the mapping should be.
				 * @param order of the mapping.
				 * @return true if the analysis has a mapping to given call with given order.
				 */
				bool isMappedTo (Call* call, CalculationOrder order);

				/**
				 * Returns true if the analysis module supports the given
				 * reduction.
				 * @param reduction to check for support.
				 * @return true iff reduction supported.
				 */
				bool supportsReduction (AnalysisModule *reduction);

				/**
				 * Returns true iff this module listens to timeouts occuring
				 * on its place of execution. This is only considered if executed
				 * on a non-application place.
				 * @return true or false.
				 */
				bool listensToTimeouts (void);

				/**
				 * Returns true if this module must not only be triggered
				 * by its input events, but also by its placement drivers regularly.
				 * @return true or false.
				 */
				bool isContinuous (void);

				/**
				 * Adds the given call as a call that this module creates.
				 * Important for wrap-across calls, as this information
				 * is needed to check module placement.
				 *
				 * The memory passed as a call is not managed by this
				 * module and must only be freed after this was destructed.
				 *
				 * @param call to add.
				 * @return true iff successful.
				 */
				bool addCallToCreate (gti::weaver::calls::Call* call);

				/**
				 * Returns the list of calls that is created by this module.
				 *
				 * Memory of the calls must not be freed by the caller.
				 *
				 * @return list of calls.
				 */
				std::list<gti::weaver::calls::Call*> getCallsCreatedByModule (void);

			protected:
				std::string myName;
				bool myIsGlobal;
				bool myIsProcessGlobal;
				bool myListensToTimeouts;
				bool myIsContinuous; /**< True if this module must not only be triggered by its input events, but also by its placement drivers regularly.*/
				std::list<AnalysisModule*> myDependencies;
				std::map <AnalysisModule*, bool> mySoftDependencies;
				std::list<AnalysisModule*> mySupportedReductions;
				std::string mySubGroupName;
				bool myIsLocalIntegrity;
				bool myIsReduction; /**< True if this module is a reduction. */
				bool myIsAddedAutomagically; /**< Automagic modules have analyses that get automagically mapped if all of their inputs are present anyways.*/
				std::list<Analysis*> myAnalyses; /**< List of analysis functions that are run on this module. */
				std::list<gti::weaver::calls::Call*> myCreates; /**< List of calls that this module creates (either wrap-across or wrap-everywhere) IMPORTANT: RIGHT NOW WE ONLY LOOK AT THIS FOR WRAPP-ACROSS!.*/

			}; /*AnalysisModule*/
		} /*namespace weaver*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // ANALYSISMODULE_H
