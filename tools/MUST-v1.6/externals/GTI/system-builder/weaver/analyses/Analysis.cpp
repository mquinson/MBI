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
 * @file Analysis.cpp
 * 		@see gti::weaver::Analysis
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>
#include <iostream>

#include "Analysis.h"

using namespace gti::weaver::analyses;

//=============================
// Analysis
//=============================
Analysis::Analysis (void)
 : Calculation (),
   myModule (NULL),
   myAnalysisFunctionName (""),
   myNeedsChannelId (false)
{
	/*Nothing to do.*/
}

//=============================
// Analysis
//=============================
Analysis::Analysis (
		std::string analysisFunctionName,
		std::vector<InputDescription*> argumentSpec,
		AnalysisGroup * group,
		AnalysisModule *module,
		bool needsChannelId)
 : Calculation (module->getName()+":"+analysisFunctionName, argumentSpec, group),
   myModule (module),
   myAnalysisFunctionName (analysisFunctionName),
   myNeedsChannelId (needsChannelId)
{
	/*Nothing to do.*/
}

//=============================
// Analysis
//=============================
Analysis::~Analysis (void)
{
	//Do not delete the analysis module this is a reference, we do not manage its memory
	myModule = NULL;
}

//=============================
// isOperation
//=============================
bool Analysis::isOperation (void)
{
	return false;
}

//=============================
// getAnalysisFunctionName
//=============================
std::string Analysis::getAnalysisFunctionName (void)
{
	return myAnalysisFunctionName;
}

//=============================
// print
//=============================
std::ostream& Analysis::print (std::ostream& out) const
{
	Calculation::print (out);
	out
		<< ", functionName=" << myAnalysisFunctionName;

	return out;
}

//=============================
// getDotNodeColor
//=============================
std::string Analysis::getDotNodeColor (void)
{
	return myModule->getDotNodeColor();
}

//=============================
// getModule
//=============================
AnalysisModule* Analysis::getModule (void)
{
	return myModule;
}

//=============================
// needsChannelId
//=============================
bool Analysis::needsChannelId (void)
{
	return myNeedsChannelId;
}

/*EOF*/
