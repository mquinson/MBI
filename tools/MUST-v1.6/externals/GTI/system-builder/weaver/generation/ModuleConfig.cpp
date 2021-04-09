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
 * @file ModuleConfig.cpp
 * 		@see gti::weaver::generation::ModuleConfig
 *
 * @author Tobias Hilbrich
 * @date 1.11.2010
 */

#include <fstream>
#include <sstream>
#include <assert.h>

#include "ModuleConfig.h"
#include "ApiCalls.h"

using namespace gti::weaver::generation;

//=============================
// ModuleConfig
//=============================
ModuleConfig::ModuleConfig (
		Level *level,
		std::string fileName)
	: GenerationBase (level, fileName, std::ios_base::out | std::ios_base::app),
	  myExtraSettings ()
{
	/*
	 * Do the generation directly in the constructor.
	 */
	out
		<< "<level index=\"" << myOrder << "\" size=\"" << mySize << "\">" << std::endl;

    //---------------------------------
	// Gather information on analyses and comms
	//---------------------------------
	if (!calculateComms())
		return;
	if (!calculateAnalyses())
		return;

	//---------------------------------
	// Prepare extra settings for level size and own level information
	//---------------------------------
	//Lets determine our own depth in the level graph
	Level* current = this;
	myOwnIndex = 0;
	while (current)
	{
	    if (!current->myInList.empty())
	    {
	        current = current->myInList.front()->target;
	        myOwnIndex++;
	    }
	    else
	    {
	        current = NULL;
	    }
	}

	//Add the level sizes based on an indexing that uses our own depth
	current = this;
	int curIndex = 0;
	while (current)
	{
	    //Add layer size
	    std::stringstream nameStream, numStream;
	    nameStream << "gti_level_" << myOwnIndex-curIndex << "_size";
	    numStream << current->getSize();

	    myExtraSettings.push_back(new Setting(nameStream.str(), numStream.str()));

	    //Add marker for mpi-ranks
	    if (!current->getPlace()->isModule() || current->getPlace()->checkRequiredApi("MPI"))
            {
                nameStream.str("");
                nameStream << "gti_level_" << myOwnIndex-curIndex << "_mpi";
                myExtraSettings.push_back(new Setting(nameStream.str(), "1"));
                
            }
	    

	    //Add distribution of ancestors
	    if (!current->myInList.empty())
	    {
            std::stringstream distribStream, dnameStream;
            dnameStream << "gti_level_" << myOwnIndex-curIndex-1 << "_" << myOwnIndex-curIndex << "_distribution";

            if (current->myInList.front()->getDistribution() == DISTRIBUTION_UNIFORM)
            {
                distribStream << "uniform";
            }
            else if (current->myInList.front()->getDistribution() == DISTRIBUTION_BY_BLOCK)
            {
                distribStream << "by-block";

                std::stringstream blockStream, blocknameStream;
                blocknameStream << "gti_level_" << myOwnIndex-curIndex-1 << "_" << myOwnIndex-curIndex << "_blocksize";
                blockStream << current->myInList.front()->getBlocksize();
                myExtraSettings.push_back(new Setting(blocknameStream.str(), blockStream.str()));
            }
            else
            {
                assert (0); //check mapping
            }

            myExtraSettings.push_back(new Setting(dnameStream.str(), distribStream.str()));
	    }

	    //Next
	    curIndex++;

	    if (!current->myInList.empty())
	        current = current->myInList.front()->target;
	    else
	        current = NULL;
	}

	//Add the own level index
	std::stringstream numStream;
	numStream << myOwnIndex;
	myExtraSettings.push_back(new Setting("gti_own_level", numStream.str()));

	//Add information on whether we have intra communication on this layer or not
	if (myIntraCommunication)
	    myExtraSettings.push_back(new Setting("gti_layer_has_intra_comm", "1"));
	else
	    myExtraSettings.push_back(new Setting("gti_layer_has_intra_comm", "0"));

	//---------------------------------
	// Print information on all the modules
	//---------------------------------
	out
		<< "\t<modules>" << std::endl;

	//== Place (if not application)
	if (myOrder != 0)
	{
	    //If we have a intra communication we need a setting to specify
	    //its module index
	    std::list<Setting*> placeSettingsTemp;
	    std::list<Setting*>::iterator setIter;

	    if (myIntraCommunication)
	        placeSettingsTemp.push_back(new Setting ("intra_strat_index", "2")); //this is always 2!

	    if (myProfiling)
	        placeSettingsTemp.push_back(new Setting ("has_profiler", "1")); //Let the place know that its last or pre last (pre last if flood control is around) child module is an implementation of I_Profiler

	    placeSettingsTemp.push_back(new Setting ("has_flood_control", "1")); //Let the place know that we also have a flood control

	    std::vector<Setting*> placeSettings;
	    placeSettings.resize(placeSettingsTemp.size());
	    int x = 0;
	    for (setIter = placeSettingsTemp.begin(); setIter != placeSettingsTemp.end(); setIter++, x++)
	        placeSettings[x] = *setIter;

		printModule (
				myPlace,
				myPlace,
				"place",
				"place",
				placeSettings);
	}
	if (myOrder == 0 && myPlace->isModule())
	{
	    std::vector<Setting*> placeSettings;
	    placeSettings.resize(0);
		printModule (
				myPlace,
				myPlace,
				"place",
				"place",
				placeSettings);
	}

	//== Incoming comm strategy + protocol (if not application)
	if (myInList.size() > 0)
	{
		printAdjacency (myInList[0], "incoming_", "protocol-down", true);
	}

	//== Wrapper
	std::string wrapperConfigName = myWrapperSourceName;
	size_t pos = wrapperConfigName.find_last_of(".");
	wrapperConfigName.replace (pos, wrapperConfigName.length()-pos, "");
	Module wrapper ("wrapper", (std::string)"lib" + wrapperConfigName);

	printModule (
			&wrapper,
			"wrapper",
			"wrapper");

	//== Receival (if not application)
	std::string receivalConfigName = myReceivalSourceName;
	pos = receivalConfigName.find_last_of(".");
	receivalConfigName.replace (pos, receivalConfigName.length()-pos, "");
	Module receival ("receival", (std::string)"lib" + receivalConfigName);

	if (myOrder != 0)
		printModule (
				&receival,
				"receival",
				"receival");

	//== Analyses
	/*
	 * The myAnalysisMap has analysis functions, we
	 * need analysis modules here, as a module may have
	 * multiple functions we must make sure to not
	 * add duplicates. We simply identify analysis modules
	 * with the id of their FIRST analysis function that we
	 * encountered.
	 *
	 * Order is crucial here, the order in which we list
	 * dependencies must reflect what the receival/wrapper
	 * generators do, so we need to stick to the order in
	 * myAnalysisMap!
	 */
	std::map<AnalysisModule*, int> usedModules; //map of analysis modules and their ids
	std::map<AnalysisModule*, int>::iterator uAnIter;
	std::list<AnalysisModule*> orderedModules;
	std::list<AnalysisModule*>::iterator orderedIter;
	std::list<Analysis*>::iterator analysisListIter;
	int panicReceiverId = -1;
	int profilerId = -1;
	int floodControlId = -1;
	/**
	 * @todo We currently allow the application layer to also have an incoming strategy, this should be generalized from the two hard coded
	 *            exception that we currently use.
	 */
	int breakEnforcerId = -1; /**< GTI internal module which we allow to receive messages from updwards on the application layer. */
	int otfxInstId = -1; /**< Online performance analysis specific module which we allow to receive messages from upwards even on the application layer.*/
	for (analysisListIter = myAnalyses.begin(); analysisListIter != myAnalyses.end(); analysisListIter++)
	{
		Analysis *a = *analysisListIter;
		int id = myAnalysisMap[a];

		//Already printed this module ?
		if (usedModules.find (a->getModule()) != usedModules.end())
			continue;

		usedModules.insert (std::make_pair(a->getModule(), id));
		orderedModules.push_back (a->getModule());

		//Remember the ids of some special analyses:
		if (a->getModule()->getName() == "PanicReceiver" && a->getModule()->getGroup()->getGroupName() == "GTI_IMPLICIT")
		    panicReceiverId = id;
		if (a->getModule()->getName() == "ProfilerMpi" && a->getModule()->getGroup()->getGroupName() == "GTI_IMPLICIT")
		    profilerId = id;
		if (a->getModule()->getName() == "FloodControl" && a->getModule()->getGroup()->getGroupName() == "GTI_IMPLICIT")
		    floodControlId = id;
		/**
		 * @todo We currently allow the application layer to also have an incoming strategy, this should be generalized from the two hard coded
		 *            exceptions that we currently use.
		 */
		if (a->getModule()->getName() == "BreakEnforcer")
		    breakEnforcerId = id;
		if (a->getModule()->getName() == "OtfxInstrumentationController")
		    otfxInstId = id;
		
		char anName[128];
		sprintf (anName, "analysis%d", id);

		printModule (
				a->getModule(),
				"analysis",
				anName);
	}

	//== Outgoing communications
	//Inter-Communication
	std::map <Adjacency*, int>::iterator commIter;

	for (commIter = myComms.begin(); commIter != myComms.end(); commIter++)
	{
		Adjacency *a = commIter->first;
		int commId = commIter->second;
		char prefix[128];
		sprintf (prefix, "outgoing%d_", commId);

		printAdjacency (a, prefix, "protocol-up", false);
	}

	//Intra-Communication
	if (myIntraCommunication)
	{
	    //Strategy
	    Module *strat = myIntraCommunication->getCommStrategyIntra();

	    printModule (
	            strat,
	            "strategy",
	            "outgoing0_strategy", //Intracommunication strategies always use id 0
	            myIntraCommunication->getStrategySettings());

	    //Protocol
	    printModule (
	            myIntraCommunication->getCommProtocol(),
	            myIntraCommunication->getCommProtocol(),
	            "protocol-intra",
	            "outgoing0_protocol",
	            myIntraCommunication->getProtocolSettings(),
	            true,
	            getOrder());
	}

	out
		<< "\t</modules>" << std::endl;

	//---------------------------------
	// Print information on module relationships
	//---------------------------------
	out
		<< "\t<relationships>" << std::endl;

	//== place
	//   -> incoming_strategy
	//   -> receival
	//   -> wrapper
	//   -> intra [if used]
	//   -> profiler [if used]
	//   -> FloodControl
	if (myOrder != 0)
	{
		out
			<< "\t\t<relationship user=\"place\">incoming_strategy</relationship>" << std::endl
			<< "\t\t<relationship user=\"place\">receival</relationship>" << std::endl;

		if (myIntraCommunication)
		    out << "\t\t<relationship user=\"place\">outgoing0_strategy</relationship>" << std::endl;

		out
			<< "\t\t<relationship user=\"place\">wrapper</relationship>" << std::endl;

		if (myProfiling)
		    out << "\t\t<relationship user=\"place\">analysis" << profilerId << "</relationship>" << std::endl;

		out << "\t\t<relationship user=\"place\">analysis" << floodControlId << "</relationship>" << std::endl;
	}
	if (myOrder == 0 && myPlace->isModule()) 
        {
                for (commIter = myComms.begin(); commIter != myComms.end() && commIter->first->getComm()->getCommStrategy()==NULL; commIter++)
                    ;
                if ( commIter != myComms.end() )
                    out << "\t\t<relationship user=\"place\">outgoing"<< commIter->second <<"_strategy</relationship>" << std::endl;
        }

	//== incoming_strategy
	//   -> incoming_protocol
	if (myOrder != 0)
	{
		out << "\t\t<relationship user=\"incoming_strategy\">incoming_protocol</relationship>" << std::endl;
	}

	//== receival/wrapper
	//   -> All strats: outgoing<i>_strategy
	//   -> incoming_strategy (If plausible)
	//   -> Intra-communication strategy if present
	//   -> All analyses: analysis<i>
	//   -> Profiler (if present)
	for (commIter = myComms.begin(); commIter != myComms.end(); commIter++)
	{
		Adjacency *a = commIter->first;
		int commId = commIter->second;
		char stratName[128];
		sprintf (stratName, "outgoing%d_strategy", commId);

		if (myOrder != 0)
			out << "\t\t<relationship user=\"receival\">" << stratName << "</relationship>" << std::endl;

		out << "\t\t<relationship user=\"wrapper\">" << stratName << "</relationship>" << std::endl;
	}

	if (myIntraCommunication)
	{
	    out << "\t\t<relationship user=\"wrapper\">outgoing0_strategy</relationship>" << std::endl;
    }

	/**
	 * @todo We currently allow the application layer to also have an incoming strategy, this should be generalized from the two hard coded
	 *            exceptions that we currently use.
	 */
	if (myInList.size() > 0) // && myInList[0]->getTarget()->getOrder() != 0)
	{
	    out << "\t\t<relationship user=\"wrapper\">incoming_strategy</relationship>" << std::endl;
	    out << "\t\t<relationship user=\"receival\">incoming_strategy</relationship>" << std::endl;
	}

	for (orderedIter = orderedModules.begin(); orderedIter != orderedModules.end(); orderedIter++)
	{
		int id = usedModules[*orderedIter];

		char anName[128];
		sprintf (anName, "analysis%d", id);

		if (myOrder != 0)
			out << "\t\t<relationship user=\"receival\">" << anName << "</relationship>" << std::endl;

		out	<< "\t\t<relationship user=\"wrapper\">" << anName << "</relationship>" << std::endl;
	}

	if (myProfiling)
	{
	    if (myOrder != 0)
	        out << "\t\t<relationship user=\"receival\">analysis" << profilerId << "</relationship>" << std::endl;
	    out << "\t\t<relationship user=\"wrapper\">analysis" << profilerId << "</relationship>" << std::endl;
	}

	//== All strats: outgoing<i>_strategy
	//   -> outgoing<i>_protocol
	for (commIter = myComms.begin(); commIter != myComms.end(); commIter++)
	{
		Adjacency *a = commIter->first;
		int commId = commIter->second;
		char stratName[128];
		char protName[128];
		sprintf (protName, "outgoing%d_protocol", commId);
		sprintf (stratName, "outgoing%d_strategy", commId);

		out	<< "\t\t<relationship user=\"" << stratName << "\">" << protName << "</relationship>" << std::endl;
	}

	if (myIntraCommunication)
	{
	    out << "\t\t<relationship user=\"outgoing0_strategy\">outgoing0_protocol</relationship>" << std::endl;
	}

	//== Analysis dependencies
	// For this loop order of modules is of no importance
	for (uAnIter = usedModules.begin(); uAnIter != usedModules.end(); uAnIter++)
	{
		AnalysisModule *module = uAnIter->first;
		std::list<AnalysisModule*> deps = module->getDependencies();
		std::list<AnalysisModule*>::iterator depIter;

		int id = uAnIter->second;

		//Here we need to stick to the order given in the dependencies
		for (depIter = deps.begin(); depIter != deps.end(); depIter++)
		{
			AnalysisModule *depMod = *depIter;

			//Check whether this is just a soft dependency, if so the dependent module is no child of this module!
			if (module->isSoftDependency(depMod))
				continue;

			assert (usedModules.find (depMod) != usedModules.end());

			int idDep = usedModules[depMod];
			out	<< "\t\t<relationship user=\"analysis" << id << "\">analysis" << idDep << "</relationship>" << std::endl;
		}
	}

	//== PanicReceiver
	// The panic receiver ha
	//   -> All strats: outgoing<i>_strategy
	//   -> Intra-communication strategy if present
	if (panicReceiverId >= 0)
	{
        for (commIter = myComms.begin(); commIter != myComms.end(); commIter++)
        {
            Adjacency *a = commIter->first;
            int commId = commIter->second;
            char stratName[128];
            sprintf (stratName, "outgoing%d_strategy", commId);
            out << "\t\t<relationship user=\"analysis"  << panicReceiverId << "\">" << stratName << "</relationship>" << std::endl;
        }

        if (myIntraCommunication)
        {
            out << "\t\t<relationship user=\"analysis" << panicReceiverId << "\">outgoing0_strategy</relationship>" << std::endl;
        }
	}

	/**
	 * @todo We currently allow the application layer to also have an incoming strategy, this should be generalized from the two hard coded
	 *            exceptions that we currently use.
	 */
	//== BreakEnforcer
	// Gets:
	//   -> All strats: outgoing<i>_strategy
	//== OtfxInstrumentationController
	// The controller gets:
	//   -> All strats: outgoing<i>_strategy
    for (commIter = myComms.begin(); commIter != myComms.end(); commIter++)
    {
        Adjacency *a = commIter->first;
        int commId = commIter->second;
        char stratName[128];
        sprintf (stratName, "outgoing%d_strategy", commId);
        
        if (otfxInstId >= 0)        
	        out << "\t\t<relationship user=\"analysis"  << otfxInstId << "\">" << stratName << "</relationship>" << std::endl;
	    if (breakEnforcerId >= 0)
	        out << "\t\t<relationship user=\"analysis"  << breakEnforcerId << "\">" << stratName << "</relationship>" << std::endl;
    }

		out
			<< "\t\t<relationship user=\"wrapper\">place</relationship>" << std::endl;
	out
	<< "\t</relationships>" << std::endl;

	//---------------------------------
	// Finalize output
	//---------------------------------
	out
		<< "\t</level>" << std::endl;
}

//=============================
// ~ModuleConfig
//=============================
ModuleConfig::~ModuleConfig (void)
{
	//Clear memory in the extra settings
    std::list<Setting*>::iterator iter;
    for (iter = myExtraSettings.begin(); iter != myExtraSettings.end(); iter++)
    {
        if (*iter)
            delete (*iter);
    }
    myExtraSettings.clear();
}

//=============================
// printAdjacency
//=============================
bool ModuleConfig::printAdjacency (
		Adjacency *a,
		std::string moduleNamePrefix,
		std::string protocolType,
		bool isDown)
{
	//Strategy
	Module *strat = a->getComm()->getCommStrategy();

	if (isDown)
		strat = a->getComm()->getCommStrategy()->getDownModule();

	if (!printModule (
			strat,
			"strategy",
			moduleNamePrefix + "strategy",
			a->getComm()->getStrategySettings()))
		return false;

	//Protocol
	std::list<Setting*> toAddSettings;
	std::stringstream stream;
	stream << a->target->getSize();
	Setting *targetSizeSetting = new Setting ("target_tier_size", stream.str());
	toAddSettings.push_back(targetSizeSetting);

	//Add distribution argument for the target layer
	if (!isDown)
	{
        std::stringstream distribStream, dnameStream;
        dnameStream << "gti_level_" << myOwnIndex << "_" << myOwnIndex+1 << "_distribution";

        if (a->getDistribution() == DISTRIBUTION_UNIFORM)
        {
            distribStream << "uniform";
        }
        else if (a->getDistribution() == DISTRIBUTION_BY_BLOCK)
        {
            distribStream << "by-block";

            std::stringstream blockStream, blocknameStream;
            blocknameStream << "gti_level_" << myOwnIndex << "_" << myOwnIndex+1 << "_blocksize";
            blockStream << a->getBlocksize();
            toAddSettings.push_back(new Setting(blocknameStream.str(), blockStream.str()));
        }
        else
        {
            assert (0); //check mapping
        }

        toAddSettings.push_back(new Setting(dnameStream.str(), distribStream.str()));
	}

	if (!printModule (
			a->getComm()->getCommProtocol(),
			a->getComm()->getCommProtocol(),
			protocolType,
			moduleNamePrefix + "protocol",
			augmentSettings(a->getComm()->getProtocolSettings(), toAddSettings),
			true,
			a->getTarget()->getOrder()))
		return false;

	if (targetSizeSetting)
	    delete (targetSizeSetting);

	return true;
}

//=============================
// printModule
//=============================
bool ModuleConfig::printModule (
	Module *module,
	std::string type,
	std::string name,
	std::vector<Setting*> settings,
	bool needsToLevel,
	int toLevel)
{
	out
		<< "\t\t<module "
		<< "type=\"" << type << "\" "
		<< "name=\"" << name << "\" "
		<< "pnmpi-module-name=\"" << module->getConfigName() << "\"";

	if (needsToLevel)
		out << " to-level=\"" << toLevel << "\"";

	out
		<< ">" << std::endl
		<< "\t\t\t<data>" << std::endl;

	std::vector<Setting*> fullSettings = augmentSettings (settings, myExtraSettings);
    for (int i = 0; i < fullSettings.size(); i++)
    {
        Setting* s = fullSettings[i];

        out << "\t\t\t\t<setting key=\"" << s->getName() << "\">" << s->getValue() << "</setting>" << std::endl;
    }

	out
		<< "\t\t\t</data>" << std::endl
		<< "\t\t</module>" << std::endl;

	return true;
}

//=============================
// printModule
//=============================
bool ModuleConfig::printModule (
	Module *module,
	Prepended *prepModule,
	std::string type,
	std::string name,
	std::vector<Setting*> settings,
	bool needsToLevel,
	int toLevel)
{
	static int prepId = 0;

	//print prepended modules
	std::list<Module*> modules = prepModule->getPrependedModules();
	std::list<Module*>::iterator i;

	for (i = modules.begin(); i != modules.end(); i++)
	{
		char temp[128];
		sprintf (temp, "prepended%d", prepId);
		prepId++;

		printModule (*i, "analysis", temp);
	}

	//print the actual module
	printModule (module, type, name, settings, needsToLevel, toLevel);

	return true;
}

//=============================
// augmentSettings
//=============================
std::vector<Setting*> ModuleConfig::augmentSettings (std::vector<Setting*> settings, std::list<Setting*> toAdd)
{
    std::vector<Setting*> ret;
    ret = settings;

    std::list<Setting*>::iterator iter;

    for (iter = toAdd.begin(); iter != toAdd.end(); iter++)
        ret.push_back(*iter);

    return ret;
}

/*EOF*/
