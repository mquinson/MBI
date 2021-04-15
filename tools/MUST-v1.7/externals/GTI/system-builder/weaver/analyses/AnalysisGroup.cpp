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
 * @file AnalysisGroup.cpp
 * 		@see gti::weaver::AnalysisGroup
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <assert.h>
#include <iostream>

#include "AnalysisGroup.h"

using namespace gti::weaver::analyses;

//=============================
// Constructor.
//=============================
AnalysisGroup::AnalysisGroup ( )
 : myLibDir(""),
   myIncDir(""),
   myGroupName (""),
   myCalculations (),
   myAnalysisModules ()
{
	/*Nothing to do*/
}

//=============================
// Constructor.
//=============================
AnalysisGroup::AnalysisGroup (std::string libDir, std::string incDir, std::string groupName)
 : myLibDir(libDir),
   myIncDir(incDir),
   myGroupName (groupName),
   myCalculations (),
   myAnalysisModules ()
{
	/*Nothing to do*/
}

//=============================
// Destructor.
//=============================
AnalysisGroup::~AnalysisGroup ( )
{
	//remove calculations
	std::list<Calculation*>::iterator i;
	for (i = myCalculations.begin(); i != myCalculations.end(); i++)
	{
		if (*i)
			delete (*i);
	}
	myCalculations.clear ();

	//remove analysis modules
	std::list<AnalysisModule*>::iterator j;
	for (j = myAnalysisModules.begin(); j != myAnalysisModules.end(); j++)
	{
		if (*j)
			delete (*j);
	}
	myAnalysisModules.clear ();
}

//=============================
// setLibDir
//=============================
void AnalysisGroup::setLibDir ( std::string new_var )
{
	myLibDir = new_var;
}

//=============================
// getLibDir
//=============================
std::string AnalysisGroup::getLibDir ( )
{
	return myLibDir;
}

//=============================
// setIncDir
//=============================
void AnalysisGroup::setIncDir ( std::string new_var )
{
	myIncDir = new_var;
}

//=============================
// getIncDir
//=============================
std::string AnalysisGroup::getIncDir ( )
{
	return myIncDir;
}

//=============================
// setGroupName
//=============================
void AnalysisGroup::setGroupName ( std::string new_var )
{
	myGroupName = new_var;
}

//=============================
// getGroupName
//=============================
std::string AnalysisGroup::getGroupName ( )
{
	return myGroupName;
}

//=============================
// getAnalyses
//=============================
std::list<Analysis*> AnalysisGroup::getAnalyses (void)
{
	std::list<Analysis*> ret;
	std::list<Calculation*>::iterator i;

	for (i = myCalculations.begin(); i != myCalculations.end (); i++)
	{
		if (!(*i)->isOperation())
			ret.push_back ((Analysis*)*i);
	}

	return ret;
}

//=============================
// getOperations
//=============================
std::list<Operation*> AnalysisGroup::getOperations (void)
{
	std::list<Operation*> ret;
	std::list<Calculation*>::iterator i;

	for (i = myCalculations.begin(); i != myCalculations.end (); i++)
	{
		if ((*i)->isOperation())
			ret.push_back ((Operation*)*i);
	}

	return ret;
}

//=============================
// addCalculation
//=============================
void AnalysisGroup::addCalculation (Calculation* calculation)
{
	assert (calculation);

	std::list<Calculation*>::iterator i;
	for (i = myCalculations.begin(); i != myCalculations.end(); i++)
	{
		if ((*i)->getName () == calculation->getName ())
			return;
	}

	myCalculations.push_back (calculation);
}

//=============================
// removeCalculation
//=============================
void AnalysisGroup::removeCalculation (Calculation *calculation)
{
	std::list<Calculation*>::iterator i;
	for (i = myCalculations.begin(); i != myCalculations.end(); i++)
	{
		if ((*i)->getName () == calculation->getName ())
		{
			myCalculations.erase(i);
			return;
		}
	}
}

//=============================
// findAnalysis
//=============================
Analysis* AnalysisGroup::findAnalysis (std::string name)
{
	std::list<Calculation*>::iterator i;
	for (i = myCalculations.begin(); i != myCalculations.end(); i++)
	{
		assert (*i);

		if ((*i)->isOperation())
			continue;

		if ((*i)->getName() == name)
			return (Analysis*)*i;
	}

	return NULL;
}

//=============================
// findOperation
//=============================
Operation* AnalysisGroup::findOperation (std::string name)
{
	std::list<Calculation*>::iterator i;
	for (i = myCalculations.begin(); i != myCalculations.end(); i++)
	{
		assert (*i);

		if (!(*i)->isOperation())
			continue;

		if ((*i)->getName() == name)
			return (Operation*)*i;
	}

	return NULL;
}

//=============================
// registerCalculation
//=============================
bool AnalysisGroup::registerCalculation (Calculation *newCalculation)
{
	std::list<Calculation*>::iterator i;
	for (i = myCalculations.begin(); i != myCalculations.end(); i++)
	{
		assert (*i);

		if ((*i)->getName() == newCalculation->getName())
			return false;
	}

	addCalculation (newCalculation);

	return true;
}

//=============================
// addAnalysisModule
//=============================
bool AnalysisGroup::addAnalysisModule (AnalysisModule *module)
{
	if (findAnalysisModule(module->getName()) != NULL)
	{
		std::cerr << " | | --> Error: there is a duplicate analysis module named \"" << module->getName() << "\" in the analysis group named \"" << getGroupName() << "\"." << std::endl;
		return false;
	}

	myAnalysisModules.push_back (module);

	return true;
}

//=============================
// removeAnalysisModule
//=============================
bool AnalysisGroup::removeAnalysisModule (AnalysisModule *module)
{
	std::list<AnalysisModule*>::iterator iter;

	for (iter = myAnalysisModules.begin(); iter != myAnalysisModules.end(); iter++)
	{
		if (*iter == module)
		{
			myAnalysisModules.erase(iter);
			return true;
		}
	}

	return false;
}

//=============================
// findAnalysisModule
//=============================
AnalysisModule* AnalysisGroup::findAnalysisModule (std::string name)
{
	std::list<AnalysisModule*>::iterator iter;

	for (iter = myAnalysisModules.begin(); iter != myAnalysisModules.end(); iter++)
	{
		if ((*iter)->getName() == name)
			return *iter;
	}

	return NULL;
}

//=============================
// getAnalysisModules
//=============================
std::list<AnalysisModule*> AnalysisGroup::getAnalysisModules (void)
{
	return myAnalysisModules;
}

//=============================
// print
//=============================
std::ostream& AnalysisGroup::print (std::ostream& out) const
{
	out << "analysisGroup={"
		<< "groupName=" << myGroupName << ", "
		<< "libDir=" << myLibDir << ", "
		<< "incDir=" << myIncDir << ", "
		<< "calculations={";

	std::list<Calculation*>::const_iterator i;
	for (i = myCalculations.begin(); i != myCalculations.end(); i++)
	{
		if (i != myCalculations.begin())
			out << ", ";
		out << "calculation={" << **i << "}";
	}

	out << "}}"; //calculations and analysis group

	return out;
}

/*EOF*/
