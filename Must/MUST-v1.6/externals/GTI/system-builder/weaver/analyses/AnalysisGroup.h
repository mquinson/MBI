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
 * @file AnalysisGroup.h
 * 		@see gti::weaver::AnalysisGroup
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef ANALYSISGROUP_H
#define ANALYSISGROUP_H

#include <string>
#include <vector>

#include "Calculation.h"
#include "Printable.h"

namespace gti
{
	namespace weaver
	{
		namespace analyses
		{
			class Calculation; /**< Forward declaration. */
			class Analysis; /**< Forward declaration. */
			class Operation; /**< Forward declaration. */
			class AnalysisModule;

			/**
			  * An analysis group with analyses and operations.
			  */
			class AnalysisGroup : virtual public Printable
			{
			public:

			  /**
			   * Invalid object constructor.
			   */
			  AnalysisGroup (void);

			  /**
			   * Constructor.
			   * @param libDir library directory for modules of this analysis group.
			   * @param incDir include directory for module interfaces of this analysis group.
			   * @param groupName name of this analysis group.
			   */
			  AnalysisGroup (std::string libDir, std::string incDir, std::string groupName);

			  /**
			   * Empty Destructor.
			   */
			  virtual ~AnalysisGroup ( );

			  /**
			   * Set the value of myLibDir
			   * @param new_var the new value of myLibDir
			   */
			  void setLibDir ( std::string new_var );

			  /**
			   * Get the value of myLibDir
			   * @return the value of myLibDir
			   */
			  std::string getLibDir (void);

			  /**
			   * Set the value of myIncDir
			   * @param new_var the new value of myIncDir
			   */
			  void setIncDir ( std::string new_var );

			  /**
			   * Get the value of myIncDir
			   * @return the value of myIncDir
			   */
			  std::string getIncDir (void);

			  /**
			   * Set the value of myGroupName
			   * @param new_var the new value of myGroupName
			   */
			  void setGroupName ( std::string new_var );

			  /**
			   * Get the value of myGroupName
			   * @return the value of myGroupName
			   */
			  std::string getGroupName (void);

			  /**
			   * Adds a calculation to this analysis group.
			   * @param calculation the calculation to add.
			   */
			  void addCalculation (Calculation* calculation);

			  /**
			   * Removes a calculation from the group.
			   * The calculation must be deleted by the caller.
			   */
			  void removeCalculation (Calculation *calculation);

			  /**
			   * Returns the list of all analyses in this
			   * analysis group.
			   * @return list of analyses.
			   */
			  std::list<Analysis*> getAnalyses (void);

			  /**
			   * Returns the list of all operations in this
			   * analysis group.
			   * @return list of operations.
			   */
			  std::list<Operation*> getOperations (void);

			  /**
			   * Searches the analysis group for an analysis with
			   * the given name.
			   * @param name of the analysis.
			   * @return pointer to the analysis if found, NULL otherwise.
			   */
			  Analysis* findAnalysis (std::string name);

			  /**
			   * Searches the analysis group for an operation with
			   * the given name.
			   * @param name of the operation.
			   * @return pointer to the operation if found, NULL otherwise.
			   */
			  Operation* findOperation (std::string name);

			  /**
			   * Called by Calculation object during construction
			   * to register the new calculation with its specified
			   * analysis group.
			   * @param newCalculation the new calculation to add.
			   * @return true if the Calculation was registered, false
			   *         if a Calculation with an equal name already
			   *         exists.
			   */
			  bool registerCalculation (Calculation *newCalculation);

			  /**
			   * Adds the given analysis module to this group.
			   * @param module, pointed to memory will be managed
			   *               by the analysis group.
			   * @return true if successful, false otherwise.
			   */
			  bool addAnalysisModule (AnalysisModule *module);

			  /**
			   * Removes the given analysis module from this group.
			   * The memory of the module is not freed and must be
			   * freed by the user.
			   */
			  bool removeAnalysisModule (AnalysisModule *module);

			  /**
			   * Finds the analysis module with the given name and
			   * returns a pointer to it.
			   * @param name of the analysis module to find.
			   * @return pointer to the module if found; NULL otherwise.
			   *
			   * The memory pointed to by the return value is still
			   * managed by the analysis group and must not be
			   * freed by the user.
			   */
			  AnalysisModule* findAnalysisModule (std::string name);

			  /**
			   * Returns all analysis modules of this analysis group.
			   * @return list of modules.
			   */
			  std::list<AnalysisModule*> getAnalysisModules (void);

			  /**
			   * Hook method for printing,
			   * in order to enable the "<<" operator.
			   * @param out ostream to use.
			   * @return ostream after printing.
			   */
			  virtual std::ostream& print (std::ostream& out) const;

			protected:
			  std::string myLibDir; /**< Library directory for modules of thie analysis group. */
			  std::string myIncDir; /**< Include directory for module interfaces of this group. */
			  std::string myGroupName; /**< Name of this group. */

			  std::list<Calculation*> myCalculations; /**< List of analyses and operations in this group. */
			  std::list<AnalysisModule*> myAnalysisModules; /**< List of analysis modules in this group. */
			};
		} /*namespace analyses*/
	} /*namespace weaver*/
} /*namespace gti*/

#endif // ANALYSISGROUP_H
