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
 * @file AnalysisModule.cpp
 * 		@see gti::weaver::AnalysisModule
 *
 * @author Tobias Hilbrich
 * @date 26.11.2010
 */

#include <assert.h>
#include <iostream>

#include "AnalysisModule.h"

using namespace gti::weaver::analyses;

//=============================
// AnalysisModule
//=============================
AnalysisModule::AnalysisModule (void)
 : Module (),
   myName (),
   myIsGlobal (false),
   myIsProcessGlobal (false),
   myDependencies (),
   mySupportedReductions (),
   mySubGroupName ("General"),
   myIsLocalIntegrity (false),
   myIsContinuous (false),
   myIsReduction(false),
   myIsAddedAutomagically (false),
   myListensToTimeouts (false),
   myAnalyses (),
   mySoftDependencies (),
   myCreates ()
{
	/*Nothing to do.*/
}

//=============================
// AnalysisModule
//=============================
AnalysisModule::AnalysisModule (
		std::string name,
		std::string moduleName,
		std::string configModuleName,
		std::string instanceType,
		std::string headerName,
		std::string incDir,
		bool isGlobal,
		bool isProcessGlobal,
		bool isLocalIntegrity,
		bool isReduction,
		bool listensToTimeouts,
		bool isContinuous,
		bool isAddedAutomagically,
		std::string subGroupName)
 : Module (moduleName, configModuleName, instanceType, headerName, incDir),
   myName (name),
   myIsGlobal (isGlobal),
   myIsProcessGlobal (isProcessGlobal),
   myDependencies (),
   mySupportedReductions (),
   mySubGroupName (subGroupName),
   myIsLocalIntegrity (isLocalIntegrity),
   myIsReduction (isReduction),
   myIsAddedAutomagically (isAddedAutomagically),
   myListensToTimeouts (listensToTimeouts),
   myIsContinuous (isContinuous),
   myAnalyses (),
   mySoftDependencies (),
   myCreates ()
{
	//Check some consistencies
	if (isLocalIntegrity && (isGlobal | isProcessGlobal))
	{
		std::cerr << " | |->Error: an analysis that is a local integrity must not be global or process global at the same time! (name=\"" << name <<  "\")" << std::endl;
		assert (0);
	}

	if (isReduction && (isLocalIntegrity || isProcessGlobal || isGlobal))
	{
		std::cerr << " | |->Error: an analysis that is a reduction must not be global, process global, or a local integrity at the same time! (name=\"" << name <<  "\")" << std::endl;
		assert (0);
	}

	if (isReduction && isAddedAutomagically)
	{
	    std::cerr << " | |->Error: an analysis that is a reduction must not be a module that uses the automagic mapping too; It is not clear what should happen here, maybe there is a natural explanation to that, but we didn't consider that one yet! (name=\"" << name <<  "\")" << std::endl;
	    assert (0);
	}
}

//=============================
// AnalysisModule
//=============================
AnalysisModule::~AnalysisModule (void)
{
	//Memory for dependencies is freed seperately, this is just a pointer
	myDependencies.clear ();

	//Clean up analyses
	std::list<Analysis*>::iterator iter;

	//// Do not delete the analyses here, their memory is managed by the analysis group to which they belong�
	//for (iter = myAnalyses.begin(); iter != myAnalyses.end(); iter++)
	//{
	//	Analysis *a = *iter;
	//	if (a) delete a;
	//}
	myAnalyses.clear();

	//Memory not managed by this module, just clear the map
	mySoftDependencies.clear();

	//Clear the list of calls that we create, we do not erase the contents as this memory is managed elsewhere
	myCreates.clear();
}

//=============================
// addDependency
//=============================
void AnalysisModule::addDependency ( AnalysisModule * add_object, bool isSoft )
{
	assert (add_object);

	//A dependency should not be a reduction
	if (add_object->isReduction())
	{
	    /**
	     * @todo
	     * It so happens that I actually exactly need this in MUST for DWaitState,
	     * I am not sure whether there is an elegant solution to this.
	     * Right now we warn but continue.
	     */
		std::cerr << " | |->Warning: you added a module dependency to a reduction. GTI will not guaranteed that the dependent reduction will be placed as you expect. You should be very certain that the reduction is placed irrespective of the presence of this dependency! Module name to which the dependency should have been added: \"" << this->getModuleName() << "\" name of the reduction \"" << add_object->getModuleName() << "\"." << std::endl;
		//assert (0);
	}

	if (isAddedAutomagically())
	{
	    std::cerr << " | |->Warning: you added a module dependency to a module that is added automagically. When GTI maps analyses of this module automagically it will not add its dependencies, so there is a good chance that you won't get what you want here! Module name of the dependency: \"" << add_object->getModuleName() << "\" name of the dependent module that uses automagick mapping \"" << this->getModuleName() << "\"." << std::endl;
	}

	//Add dependencies
	myDependencies.push_back (add_object);

	if (isSoft)
		mySoftDependencies.insert (std::make_pair(add_object, true));
}

//=============================
// removeDependency
//=============================
void AnalysisModule::removeDependency ( AnalysisModule * remove_object )
{
	std::list<AnalysisModule*>::iterator i;
	for (i = myDependencies.begin(); i != myDependencies.end(); i++)
	{
		if (*i == remove_object)
		{
			//Do not free the object, it is just a dependency
			myDependencies.erase (i);
			break;
		}
	}
}

//=============================
// getDependencies
//=============================
std::list<AnalysisModule*> AnalysisModule::getDependencies (void)
{
	return myDependencies;
}

//=============================
// addSupportedReduction
//=============================
bool AnalysisModule::addSupportedReduction ( AnalysisModule * reduction )
{
    if (isReduction())
    {
    		std::cerr << "You tried to add a supported reduction to the reduction analysis named \"" << getName() << "\"; Reductions can't support other reductions." << std::endl;
    		return false;
    }

    if (!reduction->isReduction())
    {
    		std::cerr << "You tried to add a supported reduction to the analysis named \"" << getName() << "\"; The specified analysis module was not a reduction! (Name: \"" << reduction->getName() << "\"." << std::endl;
    	    	return false;
    }

    if (isAddedAutomagically())
    {
        std::cerr << "You tried to add a supported reduction to the analysis named \"" << getName() << "\"; This module uses the automagic mapping and can't support reductions! (Name of the reduction: \"" << reduction->getName() << "\"." << std::endl;
        return false;
    }

    std::list<AnalysisModule*>::iterator iter;
    for (iter = mySupportedReductions.begin(); iter != mySupportedReductions.end(); iter++)
    {
    	    //just return if already added
    		if (*iter == reduction)
    			return true;
    }

    //add
    mySupportedReductions.push_back(reduction);

	return true;
}

//=============================
// removeAllSupportedReductions
//=============================
void AnalysisModule::removeAllSupportedReductions (void)
{
	mySupportedReductions.clear();
}

//=============================
// getSupportedReductions
//=============================
std::list<AnalysisModule*> AnalysisModule::getSupportedReductions (void)
{
	return mySupportedReductions;
}

//=============================
// isGlobal
//=============================
bool AnalysisModule::isGlobal (void)
{
	return myIsGlobal;
}

//=============================
// isProcessGlobal
//=============================
bool AnalysisModule::isProcessGlobal (void)
{
	return myIsProcessGlobal;
}

//=============================
// isReduction
//=============================
bool AnalysisModule::isReduction (void)
{
	return myIsReduction;
}

//=============================
// getName
//=============================
std::string AnalysisModule::getName (void)
{
	return myName;
}

//=============================
// print
//=============================
std::ostream& AnalysisModule::print (std::ostream& out) const
{
	out
		<< ", name=" << myName
		<< ", subGroupName=" << mySubGroupName
		<< ", isGlobal=" << myIsGlobal
		<< ", isProcessGlobal=" << myIsProcessGlobal
		<< ", module={";

	Module::print (out);

	out << "}, dependencies={";

	std::list<AnalysisModule*>::const_iterator i;
	for (i = myDependencies.begin(); i != myDependencies.end(); i++)
	{
		if (i != myDependencies.begin())
			out << ", ";
		out << (*i)->getName ();
	}
	out << "}";

	out << "analysisFunctions={";

	std::list<Analysis*>::const_iterator j;
	for (j = myAnalyses.begin(); j != myAnalyses.end(); j++)
	{
		if (j != myAnalyses.begin())
			out << ", ";
		out << "function={";
		(*j)->print(out);
		out << "}";
	}
	out << "}";

	if (myIsLocalIntegrity)
		out << ", isLocalIntegrityCheck";

	return out;
}

//=============================
// getSubGroupName
//=============================
std::string AnalysisModule::getSubGroupName (void) const
{
	return mySubGroupName;
}

//=============================
// isLocalIntegrity
//=============================
bool AnalysisModule::isLocalIntegrity (void) const
{
	return myIsLocalIntegrity;
}

//=============================
// getDotNodeColor
//=============================
std::string AnalysisModule::getDotNodeColor (void)
{
	std::string ret = "lightblue2";

	if (myIsLocalIntegrity)
		ret = "lightcyan1";

	if (myIsReduction)
		ret = "lightseagreen";

	if (myIsGlobal)
		ret = "deeppink2";

	return ret;
}

//=============================
// addAnalysis
//=============================
bool AnalysisModule::addAnalysis (
		std::string analysisFunctionName,
		std::vector<InputDescription*> argumentSpec,
		AnalysisGroup * group,
		bool needsChannelId)
{
	if (findAnalysis(analysisFunctionName) != NULL)
	{
		std::cerr << " | | --> Error: dupplicate analysis function name \"" << analysisFunctionName <<  "\" in analysis module \"" << getName()  << "\"!" << std::endl;
		return false;
	}

	myAnalyses.push_back (
			new Analysis (
					analysisFunctionName,
					argumentSpec,
					group,
					this,
					needsChannelId));

	return true;
}

//=============================
// findAnalysis
//=============================
Analysis* AnalysisModule::findAnalysis (std::string functionName)
{
	std::list<Analysis*>::iterator iter;
	for (iter = myAnalyses.begin (); iter != myAnalyses.end(); iter++)
	{
		if ((*iter)->getAnalysisFunctionName() == functionName)
			return *iter;
	}

	return NULL;
}

//=============================
// getGroup
//=============================
AnalysisGroup * AnalysisModule::getGroup (void)
{
	if (myAnalyses.empty())
		return NULL;

	Analysis* a = myAnalyses.front();

	return a->getGroup();
}

//=============================
// getAnalyses
//=============================
std::list <Analysis*> AnalysisModule::getAnalyses (void)
{
	return myAnalyses;
}

//=============================
// isMappedTo
//=============================
bool AnalysisModule::isMappedTo (Call* call, CalculationOrder order)
{
	std::list<Analysis*>::iterator iter;

	for (iter = myAnalyses.begin(); iter != myAnalyses.end(); iter++)
	{
		Analysis * a = *iter;

		std::vector<Mapping*> mappings = a->getCallMappings();

		for (int i = 0; i < mappings.size(); i++)
		{
			Mapping *m = mappings[i];

			if (m->getApiCall() == call && m->getOrder() == order)
				return true;
		}
	}
	return false;
}

//=============================
// supportsReduction
//=============================
bool AnalysisModule::supportsReduction (AnalysisModule *reduction)
{
	assert (reduction->isReduction());

	std::list<AnalysisModule*>::iterator iter;

	for (iter = mySupportedReductions.begin(); iter != mySupportedReductions.end(); iter++)
	{
		AnalysisModule* mod = *iter;

		if (mod == reduction)
			return true;
	}

	return false;
}

//=============================
// isSoftDependency
//=============================
bool AnalysisModule::isSoftDependency (AnalysisModule* dependency)
{
	if (mySoftDependencies.find (dependency) != mySoftDependencies.end())
		return true;

	return false;
}

//=============================
// listensToTimeouts
//=============================
bool AnalysisModule::listensToTimeouts (void)
{
    return myListensToTimeouts;
}

//=============================
// isContinuous
//=============================
bool AnalysisModule::isContinuous (void)
{
	return myIsContinuous;
}

//=============================
// addCallToCreate
//=============================
bool AnalysisModule::addCallToCreate (gti::weaver::calls::Call* call)
{
    std::list<gti::weaver::calls::Call*>::iterator iter;
    for (iter = myCreates.begin(); iter != myCreates.end(); iter++)
    {
        //already added
        if (*iter == call)
            return true;
    }

    myCreates.push_back(call);

    return true;
}

//=============================
// getCallsCreatedByModule
//=============================
std::list<gti::weaver::calls::Call*> AnalysisModule::getCallsCreatedByModule ()
{
    return myCreates;
}

//=============================
// isAddedAutomagically
//=============================
bool AnalysisModule::isAddedAutomagically (void) const
{
    return myIsAddedAutomagically;
}

/*EOF*/
