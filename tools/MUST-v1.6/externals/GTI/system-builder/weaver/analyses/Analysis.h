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
 * @file Analysis.h
 * 		@see gti::weaver::Analysis
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <string>
#include <vector>

#include "Printable.h"
#include "Calculation.h"
#include "Module.h"
#include "AnalysisModule.h"

using namespace gti::weaver::modules;

namespace gti
{
	namespace weaver
	{
		namespace analyses
		{
			class AnalysisGroup;
			class Analysis;
			class AnalysisModule;

			/**
			 * Specification for an analysis function which is one
			 * out of possibly multiple analysis functions in an
			 * AnalysisModule.
			 */
			class Analysis : public Calculation, virtual public Printable
			{
			public:
				friend std::ostream& operator<<(std::ostream& out, const Analysis& l);

				/**
				 * Empty Constructor
				 */
				Analysis ( );

				/**
				 * Proper Constructor
				 */
				Analysis (
						std::string analysisFunctionName,
						std::vector<InputDescription*> argumentSpec,
						AnalysisGroup * group,
						AnalysisModule* module,
						bool needsChannelId = false);

				/**
				 * Empty Destructor
				 */
				virtual ~Analysis ( );

				/**
				 * Returns true if this is an operation
				 * and false if it is an analysis.
				 */
				bool isOperation (void);

				/**
				 * Returns the name of the function that invokes
				 * this analysis.
				 */
				std::string getAnalysisFunctionName (void);

				/**
				 * Hook method for printing,
				 * in order to enable the "<<" operator.
				 * @param out ostream to use.
				 * @return ostream after printing.
				 */
				virtual std::ostream& print (std::ostream& out) const;

				/**
				 * Returns a color that should be used to fill the dot node
				 * for this calculation. (The color should represent the
				 * calculation type as well as key properties of it.)
				 * @return DOT color name.
				 */
				virtual std::string getDotNodeColor (void);

				/**
				 * Returns the module for this analysis.
				 * @return pointer to the module.
				 *
				 * The memory pointed to by the return value is
				 * still managed by Analyses namespace and must
				 * not be freed by the user.
				 */
				AnalysisModule* getModule (void);

				/**
				 * Returns true if needsChannelId was set to true in
				 * the constructor and false otherwise.
				 * @return true or false.
				 */
				bool needsChannelId (void);

			protected:
				AnalysisModule *myModule; /**< Analysis module to which this analysis belongs. */
				std::string myAnalysisFunctionName; /**< Name of the analysis function within the module, used to identify and to call the analysis in actual code. */
				bool myNeedsChannelId;

			}; /*Analysis*/
		} /*namespace weaver*/
	} /*namespace weaver*/
} /*namespace gti*/
#endif // ANALYSIS_H
