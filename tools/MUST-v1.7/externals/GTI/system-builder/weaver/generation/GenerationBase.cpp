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
 * @file GenerationBase.cpp
 * 		@see gti::weaver::generation::GenerationBase
 *
 * @author Tobias Hilbrich
 * @date 13.08.2010
 */

#include "GenerationBase.h"
#include "ApiCalls.h"

#include <fstream>
#include <assert.h>

using namespace gti::weaver::generation;

//=============================
// GenerationBase
//=============================
GenerationBase::GenerationBase (
		Level *level,
		std::string fileName,
		std::ios_base::openmode mode)
	: out (),
	  myHeaders (),
	  myComms (),
	  myAnalysisMap ()
{
	//Copy attributes from level
	myOrder = level->myOrder;
	mySize = level->mySize;
	myPlace = level->myPlace;

	myInList = level->myInList;
	myOutList = level->myOutList;
	myAnalyses = level->myAnalyses;
	myAnalysisModules = level->myAnalysisModules;

	myCallPropertiesPre = level->myCallPropertiesPre;
	myCallPropertiesPost = level->myCallPropertiesPost;

	myWrapperSourceName = level->myWrapperSourceName;
	myReceivalSourceName = level->myReceivalSourceName;

	myReductionForwards = level->myReductionForwards;

	myIntraCommunication = level->myIntraCommunication;
	myProfiling = level->myProfiling;

	//Open the output file
	out.open (fileName.c_str(), mode);
}

//=============================
// ~GenerationBase
//=============================
GenerationBase::~GenerationBase (void)
{
	/*
	 * Do not attempt to free any memory here, we are just using copies
	 * of another place, this place will free all the memory of any
	 * allocated objects.
	 */
	myInList.clear();
	myOutList.clear();
	myAnalyses.clear();
	myAnalysisModules.clear();
	myCallPropertiesPre.clear();
	myCallPropertiesPost.clear();
	myReductionForwards.clear();

	myIntraCommunication = NULL;

	out.close();
}

//=============================
// writeSettings
//=============================
bool GenerationBase::writeSettings (
		std::string sourceFileName,
		std::string headerFileName,
		std::string logFileName)
{
	out
		<< "\t<settings>" << std::endl
		<< "\t\t<output-dir>" << "./" << "</output-dir>" << std::endl
		<< "\t\t<source-filename-out>" << sourceFileName << "</source-filename-out>" << std::endl
		<< "\t\t<header-filename-out>" << headerFileName << "</header-filename-out>" << std::endl
		<< "\t\t<log-filename-out>" << logFileName << "</log-filename-out>" << std::endl
		<< "\t</settings>" << std::endl;
	return true;
}

//=============================
// calculateHeaders
//=============================
bool GenerationBase::calculateHeaders (bool forWrapper)
{
	//a) GTI comm strategies
	myHeaders.insert (std::make_pair("I_CommStrategyUp.h", false));
	myHeaders.insert (std::make_pair("I_CommStrategyDown.h", false));

	if (forWrapper)
	    myHeaders.insert (std::make_pair("I_CommStrategyIntra.h", false));

	//b) Some basic headers
	myHeaders.insert (std::make_pair("pnmpimod.h", true));
	myHeaders.insert (std::make_pair("assert.h", true));
	myHeaders.insert (std::make_pair("stdio.h", true));
	myHeaders.insert (std::make_pair("iostream", true));

	//d) API headers
	if (forWrapper)
	{
		std::list<std::string> tempHeaders = ApiCalls::getInstance()->getAllApiHeaders();
		std::list<std::string>::iterator tempIter;
		for (tempIter = tempHeaders.begin(); tempIter != tempHeaders.end(); tempIter++)
			myHeaders.insert(std::make_pair(*tempIter, true));
	}

	//d) Analysis headers
	std::list<AnalysisModule*>::iterator anModIter;
	for (anModIter = myAnalysisModules.begin(); anModIter != myAnalysisModules.end(); anModIter++)
	{
		AnalysisModule* anModule = *anModIter;
		if (anModule->getIncDir() != "")
			myHeaders.insert (std::make_pair(anModule->getIncDir() + "/" + anModule->getHeaderName(), false));
		else
			myHeaders.insert (std::make_pair(anModule->getHeaderName(), false));
	}

	//e) Operation headers
	if (!forWrapper)
		return true; //receiveal/forward needs no operations

	CalculationOrder orders[] = {ORDER_PRE, ORDER_POST};
	const int num_orders=2;
	for (int i = 0; i < num_orders; i++)
	{
		std::map<Call*, CallProperties*>* props = getCallPropertiesForOrder (orders[i]);
		std::map<Call*, CallProperties*>::iterator propIter;

		for (propIter = props->begin(); propIter != props->end(); propIter++)
		{
			CallProperties* prop = propIter->second;
			std::list<std::pair<Operation*, int> > ops = prop->getMappedOperations ();
			std::list<std::pair<Operation*, int> >::iterator opIter;

			for (opIter = ops.begin(); opIter != ops.end(); opIter++)
			{
				Operation* op = opIter->first;
				std::list<std::string> tempHeaders = op->getExtraHeaders();
				std::list<std::string>::iterator headerIter;

				for (headerIter = tempHeaders.begin();headerIter != tempHeaders.end();headerIter++)
				{
					if (op->getGroup()->getIncDir() != "")
						myHeaders.insert (std::make_pair(op->getGroup()->getIncDir() + "/" + *headerIter, false));
					else
						myHeaders.insert (std::make_pair(*headerIter, false));
				}
			}
		}
	}

	return true;
}

//=============================
// printHeaders
//=============================
bool GenerationBase::printHeaders (void)
{
	std::map<std::string, bool>::iterator hIter;

	out	<< "\t<headers>" << std::endl;
	for (hIter = myHeaders.begin(); hIter != myHeaders.end(); hIter++)
	{
		std::string isSystem = "no";
		if (hIter->second)
			isSystem = "yes";

		out
			<< "\t\t<header is-system=\""
			<< isSystem
			<< "\">"
			<< hIter->first
			<< "</header>"
			<< std::endl;
	}

	out << "\t</headers>" << std::endl;

	return true;
}

//=============================
// calculateComms
//=============================
bool GenerationBase::calculateComms (void)
{
    //IMPORTANT: we reserve communication id "0" for intra communication and id "1" for downwards communication!
	for (int i = 0; i < myOutList.size(); i++)
		myComms.insert (std::make_pair(myOutList[i], i+2));

	return true;
}

//=============================
// printComms
//=============================
bool GenerationBase::printComms (bool includeIntraComm)
{
	std::map <Adjacency*, int>::iterator commIter;

	out << "\t<communications>" << std::endl;

	//Intercommunication
	for (commIter = myComms.begin(); commIter != myComms.end(); commIter++)
	{
		CommStrategy *comm = commIter->first->getComm()->getCommStrategy();

		out
			<< "\t\t<communication id=\""
			<< commIter->second
			<< "\">"
			<< comm->getModuleName()
			<< "</communication>"
			<< std::endl;
	}

	//Intracommunication
	if (includeIntraComm)
	{
        if (myIntraCommunication)
        {
            CommStrategyIntra *comm = myIntraCommunication->getCommStrategyIntra();

            out
            << "\t\t<communication id=\""
            << 0 //IMPORTANT: intra communication is always id 0
            << "\" is-intra=\"yes\">"
            << comm->getModuleName()
            << "</communication>"
            << std::endl;
        }
	}

    //Downwards communication
	/**
	 * Nov. 26, 2013: Enabled generation of downwards communication protocol-strategy; This does not hurts yet.
	 */
	if (myInList.size() > 0) //&& myInList[0]->getTarget()->getOrder() != 0
	{
        CommStrategy *comm = myInList[0]->getComm()->getCommStrategy();

        out
            << "\t\t<communication id=\""
            << 1 //IMPORTANT: downwards communication is always id 1
            << "\" is-down=\"yes\">"
            << comm->getDownModule()->getModuleName()
            << "</communication>"
            << std::endl;
	}

	out << "\t</communications>" << std::endl;

	return true;
}

//=============================
// calculateAnalyses
//=============================
bool GenerationBase::calculateAnalyses(void)
{
	/* We map an ID to each analysis.
	 *  The generators (receival/wrapper) will deal with cases where one
	 *  analysis module has multiple analysis functions.
	 */
	std::list<Analysis*>::iterator tempIter;
	int iAnalysis = 0;
	for (tempIter = myAnalyses.begin(); tempIter != myAnalyses.end(); tempIter++, iAnalysis++)
		myAnalysisMap.insert (std::make_pair(*tempIter, iAnalysis));

	return true;
}

//=============================
// printAnalyses
//=============================
bool GenerationBase::printAnalyses(void)
{
	std::list<Analysis*>::iterator analysisListIter;

	out << "\t<analyses>" << std::endl;

	for (analysisListIter = myAnalyses.begin(); analysisListIter != myAnalyses.end(); analysisListIter++)
	{
		Analysis* analysis = *analysisListIter;

		std::string isReductionStr = "no";
		if (analysis->getModule()->isReduction())
			isReductionStr="yes";

		std::string needsChannelIdStr = "no";
		if (analysis->needsChannelId ())
			needsChannelIdStr = "yes";

		std::string isContinuousStr = "no";
		if (analysis->getModule()->isContinuous())
			isContinuousStr = "yes";

		std::string listensToTimeoutsStr = "no";
		if (analysis->getModule()->listensToTimeouts())
		    listensToTimeoutsStr = "yes";

		out
			<< "\t\t<analysis reduction=\"" << isReductionStr << "\" needs-channel-id=\"" << needsChannelIdStr << "\" listens-to-timeouts=\"" << listensToTimeoutsStr << "\" continuous=\"" << isContinuousStr << "\">"
			<< std::endl
			<< "\t\t\t<analysis-id>analysis_"
			<< myAnalysisMap[analysis]
			<< "</analysis-id>"
			<< std::endl
			<< "\t\t\t<analysis-name>"
			<< analysis->getModule()->getName()
			<< "</analysis-name>"
			<< std::endl
			<< "\t\t\t<analysis-datatype>"
			<< analysis->getModule()->getInstanceType()
			<< "</analysis-datatype>"
			<< std::endl
			<< "\t\t\t<analysis-function>"
			<< analysis->getAnalysisFunctionName()
			<< "</analysis-function>"
			<< std::endl
			<< "\t\t</analysis>"
			<< std::endl;
	}

	//analysis (analysis-id, analysis-name, analysis-datatype, analysis-function)

	out << "\t</analyses>" << std::endl;

	return true;
}

//=============================
// printExecAnalyses
//=============================
void GenerationBase::printExecAnalyses (Call *call, bool integrity, CalculationOrder order, bool isForWrapper)
{
	std::map <int, std::map<Mapping*, Analysis*> > execMappings; //intraCallOrder mapped to a list of  Analyses and their Mapping, used to sort according to the intra Call order (the inner map is used for cases where the same intra call order is used multiple times)
	std::map <int, std::map<Mapping*, Analysis*> >::iterator execIter;

	bool hasReduction = false;
	AnalysisModule *reductionMod = NULL;

	//Find all suitable mappings and add them to "execMappings"
	std::list<Analysis*>::iterator analysisIter;
	for (analysisIter = myAnalyses.begin(); analysisIter != myAnalyses.end(); analysisIter++)
	{
		Analysis *analysis = *analysisIter;

		//Only run integrities if it is actually time for them ...
		if (analysis->getModule()->isLocalIntegrity() != integrity)
			continue;

		//Do not run reductions on wrappers
		// a) on a wrapper there only comes info from one process -> thus no inter process reduction possible
		// b) the wrapper must not intercept its own wrapp-everywhere call (infinte recursion possible)
		if (isForWrapper && analysis->getModule()->isReduction())
			continue;

		std::list<Mapping*> mappings = analysis->getMappingsForCall (call);
		std::list<Mapping*>::iterator mapIter;

		for (mapIter = mappings.begin(); mapIter != mappings.end(); mapIter++)
		{
			Mapping *m = *mapIter;

			if (m->getOrder() != order)
				continue;

			//Do we have a reduction ? If so store it
			if (analysis->getModule()->isReduction())
			{
				//There must be at most one reduction on one event, so we assert if this doesn't holds
				if (hasReduction && analysis->getModule() != reductionMod)
				{
					std::cerr
					<< "Error: two reductions are mapped to a single call and order, which must not happen according to the rules for reductions!"
					<< " (reduction1=" << reductionMod->getModuleName() << ", reduction2=" << analysis->getModule()->getModuleName()
					<< ", call=" << call->getName() << ") (" << __FILE__ << "@" << __LINE__ << ")"
					<< std::endl;
					assert (0);
				}

				reductionMod = analysis->getModule();
				hasReduction = true;
			}

			//Reject automagic analysis for which we lack some input (or if this is not for a wrapper)
			if (analysis->getModule()->isAddedAutomagically())
			{
			    //These get only executed in wrappers
			    if (!isForWrapper)
			        continue;

			    //Do we have all the inputs for this?
			    std::map<Call*, CallProperties*>* allProps = getCallPropertiesForOrder(order);
			    std::map<Call*, CallProperties*>::iterator cProps = allProps->find(call);
			    if (cProps == allProps->end())
			        continue;

			    //Do we have all the inputs?
			    std::vector<Input*> mInputs = m->getArgumentInputs();
			    std::list<Input*> mInputsList (mInputs.begin(), mInputs.end());
			    if (!cProps->second->usedOrReceiveArgsContain(mInputsList))
			        continue;
			}

			//add to execMappings
			if (execMappings.find (m->getIntraCallOrder()) != execMappings.end())
			{
				//Add to existing intra order
				execMappings[m->getIntraCallOrder()].insert (std::make_pair(m, analysis));
			}
			else
			{
				//New intra order
				std::map <Mapping*, Analysis*> temp;
				temp.insert (std::make_pair(m, analysis));
				execMappings.insert (std::make_pair (m->getIntraCallOrder(), temp));
			}
		}
	}

	//Loop over execMappings and execute them
	for (execIter = execMappings.begin(); execIter != execMappings.end(); execIter++)
	{
		std::map <Mapping*, Analysis*>::iterator mappIter;

		for (mappIter = execIter->second.begin(); mappIter != execIter->second.end(); mappIter++)
		{
			Mapping* m = mappIter->first;
			Analysis* analysis = mappIter->second;

			bool supportsReduction=false;
			if (hasReduction && analysis->getModule()->supportsReduction(reductionMod))
				supportsReduction=true;

			//print the execution
			printMappingExec (m, myAnalysisMap[analysis], supportsReduction);
		}
	}
}

//=============================
// printMappingExec
//=============================
void GenerationBase::printMappingExec (Mapping *m, int analysisId, bool supportsReduction)
{
	out
		<< "\t\t\t\t<exec-analysis";

	if (supportsReduction)
		out << " supports-reduction=\"yes\"";

	out
		<<">"
		<< std::endl
		<< "\t\t\t\t\t<analysis-id>"
		<< "analysis_"
		<< analysisId
		<< "</analysis-id>"
		<< std::endl
		<< "\t\t\t\t\t<inputs>"
		<< std::endl;

	std::vector<Input*> inputs = m->getArgumentInputs();
	for (int i = 0; i < inputs.size(); i++)
	{
		Input* input = inputs[i];

		out << "\t\t\t\t\t\t<input>" << input->getName() << "</input>" << std::endl;
	}

	out
		<< "\t\t\t\t\t</inputs>"
		<< std::endl
		<< "\t\t\t\t</exec-analysis>"
		<< std::endl;
}

//=============================
// printRecord
//=============================
void GenerationBase::printRecord (Call *call, int uid, std::list<Input*> inputs)
{
	out	<< "\t\t\t\t\t\t<record uid=\"" << uid << "\">"	<< std::endl;

	//add a "element"/"array-element" node for each input
	std::list<Input*>::iterator inputIter;
	for (inputIter = inputs.begin(); inputIter != inputs.end(); inputIter++)
	{
		Input* input = *inputIter;
		if (input->isArrayInput())
			out << "\t\t\t\t\t\t\t<array-element>" << std::endl;
		else
			out << "\t\t\t\t\t\t\t<element>" << std::endl;

		out
			<< "\t\t\t\t\t\t\t\t<name>" << input->getName() << "</name>"  << std::endl
			<< "\t\t\t\t\t\t\t\t<type>" << input->getType() << "</type>"  << std::endl
			<< "\t\t\t\t\t\t\t\t<from-call>" << call->getName() << "</from-call>"  << std::endl
			<< "\t\t\t\t\t\t\t\t<as-arg>" << input->getName() << "</as-arg>"  << std::endl;

		if (input->isArrayInput())
		{
			out
				<< "\t\t\t\t\t\t\t\t<length-argument>" << input->getLenName() << "</length-argument>"  << std::endl
				<< "\t\t\t\t\t\t\t</array-element>" << std::endl;
		}
		else
		{
			out << "\t\t\t\t\t\t\t</element>" << std::endl;
		}
	}

	out	<< "\t\t\t\t\t\t</record>"	<< std::endl;
}

//=============================
// printForwardingForCall
//=============================
void GenerationBase::printForwardingForCall (
  Call *call,
  CalculationOrder order,
  bool excludeIncomingRecord,
  bool allowIntraCommunication,
  bool disableInterCommunication)
{
	out
		<< "\t\t\t\t<forwarding>"
		<< std::endl
		<< "\t\t\t\t\t<records>"
		<< std::endl;

	std::map <int , std::list<int> > recordedUids; /*Map that stores which uids need to be forwarded to what communication ids (also used to avoid creating the same record multiple times)*/

	//Loop over all adjacent levels (intra comm) and use the extra iterations (+2) to look at intra communication and at downwards communication
	for (int i = 0; i < myOutList.size() + 2; i++)
	{
	    CallProperties* prop;
	    int commIdToUse;

	    if (i < myOutList.size())
	    {
	        if (disableInterCommunication)
	            continue;

	        //A) Inter layer communication
	        Level* target = myOutList[i]->getTarget();
	        std::map<Call*, CallProperties*> *props = target->getCallPropertiesForOrder(order);

	        //Is this a wrapp-down call?
	        if (call->isWrappedDown())
	            continue;

	        //has target properties for this call ?
	        if (props->find(call) == props->end())
	            continue;

	        prop = (*props)[call];

	        commIdToUse = myComms[myOutList[i]];
	    }
	    else if (i == myOutList.size())
	    {
	        //B) Intra layer communication
	        if (!allowIntraCommunication)
	            continue;

	        //extra iteration to investigate intra communication
	        if (!myIntraCommunication || !call->isWrappedAcross())
	            continue;

	        //The properties in question are our own ones
	        std::map<Call*, CallProperties*> *props = getCallPropertiesForOrder(order);

	        if (props->find(call) == props->end())
	            continue;

	        prop = (*props)[call];

	        if (!prop)
	            continue;

	        //The communication id is 0 for intra communication
	        commIdToUse = 0;
	    }
	    else
	    {
	        //C) Down communication
	        if (myInList.size() == 0)
	            continue; //No one to communicate with

			/**
	         * @todo Exception for two special calls to be allowed to progress towards the application layer.
	         *            This should be generalized at some point.
	         */
	        if (myInList[0]->getTarget()->getOrder() == 0 && 
	            call->getName() != "gtiBroadcastBreak" && /**< This one is used to pause the application as to avoid OOM situations for th MUST use case.*/
	            call->getName() != "toggleInstrumentation"/**< This one is used for online performance analysis to enable application steering.*/ )
	            continue; //We can't send down to the application (usually ;))

	        if (!call->isWrappedDown())
	            continue;

	        Level* target = myInList[0]->getTarget();

	        std::map<Call*, CallProperties*> *props = target->getCallPropertiesForOrder(order);

	        //has target properties for this call ?
	        if (props->find(call) == props->end())
	            continue;

	        prop = (*props)[call];

	        if (!prop)
	            continue;

	        //The communication id is 1 for intra communication
	        commIdToUse = 1;
	    }

		std::list<Input*> inputs = prop->getArgsToReceive();

		//      Here we need to check whether this uid (from prop)
		//      is already in the recordedUids map, if so skip the
		//      generation of an equal record !
		//      Otherwise add to map!
		if (recordedUids.find(prop->getInRecordUniqueId()) != recordedUids.end())
		{
			recordedUids[prop->getInRecordUniqueId()].push_back(commIdToUse);
			continue;
		}
		std::list <int> tempList;
		tempList.push_back (commIdToUse);
		recordedUids.insert (
				std::make_pair (
						prop->getInRecordUniqueId(),
						tempList));

		/*
		 * If desired check whether the record for this forwarding is equal
		 * to the record received for this call and order, if so there is no
		 * need to print it again.
		 * Usage: for generation of receival wherere the incoming record
		 * 	      is already specified.
		 */
		if (excludeIncomingRecord)
		{
			std::map<Call*, CallProperties*> *props = getCallPropertiesForOrder(order);
			CallProperties *ownProp = (*props)[call];
			if (!ownProp) continue;
			if (ownProp->getInRecordUniqueId() == prop->getInRecordUniqueId())
				continue;
		}

		//we need a forward entry in the XML
		if (!disableInterCommunication || commIdToUse == 0)
		    printRecord (call, prop->getInRecordUniqueId(), inputs);
	}

	//forwards, forward, uid~commId
	out
		<< "\t\t\t\t\t</records>"
		<< std::endl
		<< "\t\t\t\t\t<forwards>"
		<< std::endl;

	std::map <int , std::list<int> >::iterator recordedUidIter;
	for (recordedUidIter = recordedUids.begin(); recordedUidIter != recordedUids.end(); recordedUidIter++)
	{
		int uid = recordedUidIter->first;
		std::list<int>::iterator listIter;

		for (listIter = recordedUidIter->second.begin(); listIter != recordedUidIter->second.end(); listIter++)
		{
			int commId = *listIter;
			std::string reducableStr = "no";

			if (disableInterCommunication && commId != 0)
			    continue;

			//Determine whether this forward may be replaced by a successful reduction -> if so set reducableStr to "yes"
			std::list<int> reducableChannels;
			std::list<int>::iterator intIter;
			if (myReductionForwards.find(std::make_pair(call, order)) !=  myReductionForwards.end())
				reducableChannels = myReductionForwards[std::make_pair(call, order)];

			for (intIter = reducableChannels.begin(); intIter != reducableChannels.end(); intIter++)
			{
				if (myComms[myOutList[*intIter]] == commId)
				{
					reducableStr = "yes";
					break;
				}
			}

			//Write the forward to the XML
			out << "\t\t\t\t\t\t<forward reducable=\"" << reducableStr << "\"><record-uid>" << uid << "</record-uid><comm-id>" << commId << "</comm-id></forward>" << std::endl;
		}
	}
	//clear map, for post
	recordedUids.clear ();

	out
		<< "\t\t\t\t\t</forwards>" << std::endl
		<< "\t\t\t\t</forwarding>" << std::endl;
}

/*EOF*/
