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
 * @file Wrapper.cpp
 * 		@see gti::weaver::Wrapper
 *
 * @author Tobias Hilbrich
 * @date 11.08.2010
 */

#include <fstream>

#include "Wrapper.h"
#include "ApiCalls.h"

using namespace gti::weaver::generation;

//=============================
// Wrapper
//=============================
Wrapper::Wrapper (
		Level *level,
		std::string fileName,
		std::string sourceFileName,
		std::string headerFileName,
		std::string logFileName)
	: GenerationBase (level, fileName)
{
	/*
	 * Do the generation directly in the constructor.
	 */
	out
		<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
		<< "<wrapper-specification order=\"" << myOrder << "\">" << std::endl;

	if (!writeSettings(sourceFileName, headerFileName, logFileName))
		return;

	//---------------------------------
	//create a list of all headers and add it to the XML
	//---------------------------------
	if (!calculateHeaders (true))
		return;
	if (!printHeaders ())
		return;

	//---------------------------------
	//create list of all communication modules and add to XML
	//---------------------------------
	if (!calculateComms())
		return;
	if (!printComms(true))
		return;

	//---------------------------------
	//create list of analysis modules and add it to XML
	//---------------------------------
	if (!calculateAnalyses())
		return;
	if (!printAnalyses())
		return;

	//---------------------------------
	//write out information on each wrapped called
	//---------------------------------
	//list of calls that need to be wrapped
	std::map<Call*,bool> callsToWrap;
	std::map<Call*,bool>::iterator callIter;

	out << "\t<calls>" << std::endl;

	CalculationOrder orders[] = {ORDER_PRE, ORDER_POST};
	const int num_orders=2;
	for (int i = 0; i < num_orders; i++)
	{
		std::map<Call*, CallProperties*>* props = getCallPropertiesForOrder (orders[i]);
		std::map<Call*, CallProperties*>::iterator propIter;

		for (propIter = props->begin(); propIter != props->end(); propIter++)
		{
			CallProperties* prop = propIter->second;
			if (prop->needsWrapper())
				callsToWrap.insert (std::make_pair (propIter->first, true));
		}
	}

	for (callIter = callsToWrap.begin(); callIter != callsToWrap.end(); callIter++)
	{
		Call* call = callIter->first;
		printWrappGenCall (call);
	}

	out
		<< "\t</calls>" << std::endl
		<< "</wrapper-specification>" << std::endl;
}

//=============================
// ~Wrapper
//=============================
Wrapper::~Wrapper (void)
{

}

//=============================
// printWrappGenCall
//=============================
void Wrapper::printWrappGenCall (Call* call)
{
    //0) ----------- Can we provide this call at all ? -----------
    //We can't provide if it is a wrap across call while we don't have intra layer commnication
    if (call->isWrappedAcross() && !myIntraCommunication)
        return;

	//1) ----------- basic stuff -----------
	out
		<< "\t\t<call";

	if (call->isFinalizer())
		out << " is-finalizer=\"yes\"";

	if (call->isLocalFinalizer())
	    out << " is-local-finalizer=\"yes\"";

	if (call->isWrappedAcross())
	    out << " is-wrap-across=\"yes\"";

	if (call->isWrappedDown())
	    out << " is-wrap-down=\"yes\"";
        
        if (call->isCallback())
	    out << " callback=\"yes\"";

	if (call->isHook())
	    out << " hook=\"yes\"";
        
	out
	    << ">"
		<< std::endl
		<< "\t\t\t<return-type>"
		<< call->getReturnType()
		<< "</return-type>"
		<< std::endl
		<< "\t\t\t<call-name>"
		<< call->getName()
		<< "</call-name>"
		<< std::endl
		<< "\t\t\t<arguments>"
		<< std::endl;

	//2) ----------- Arguments -----------
	std::vector<Argument*> args = call->getArguments ();
	for (int i = 0; i < args.size(); i++)
	{
		out
			<< "\t\t\t\t<argument typeAfterArg=\"" << args[i]->getTypeAfterArg() << "\">"
			<< "<type>"
			<< args[i]->getType()
			<< "</type><arg>"
			<< args[i]->getName()
			<< "</arg>"
			<< "</argument>"
			<< std::endl;
	}

	out
		<< "\t\t\t</arguments>"
		<< std::endl
		<< "\t\t\t<pre>"
		<< std::endl;

	//3alpha) -- Do we have an integrity ?
	bool hasPreIntegrity = false;
	bool hasPostIntegrity = false;
	std::list<Analysis*>::iterator analysisIter;
	for (analysisIter = myAnalyses.begin(); analysisIter != myAnalyses.end(); analysisIter++)
	{
		Analysis *analysis = *analysisIter;

		//Only run integrities if it is actually time for them ...
		if (!analysis->getModule()->isLocalIntegrity())
			continue;

		if (analysis->getModule()->isMappedTo(call, ORDER_PRE))
			hasPreIntegrity = true;

		if (analysis->getModule()->isMappedTo(call, ORDER_POST))
			hasPostIntegrity = true;
	}

	//3) ----------- Pre -----------
	std::list<std::pair<Operation*, int> > execOps;
	bool execAnalyses = true;

	if (call->isWrappedAcross())
	    execAnalyses = false; //We do not execute analyses in wrap across call, the intention is that the analyses mapped to them are executed on the remote side instead! However, we still call operations here to perform any necessary data preparations!

	//Integrities
	if (hasPreIntegrity)
	{
		//Integrity input Ops
		printIntegritySourcePieces (call, ORDER_PRE, &execOps);

		//Pre Integrity
		if (execAnalyses)
		    printExecAnalyses (call, true, ORDER_PRE, true);
	}

	//Pre Ops
	printSourcePieces (call, false, ORDER_PRE, &execOps);

	//Pre Analyses
	if (execAnalyses)
	    printExecAnalyses (call, false, ORDER_PRE, true);

	//forwarding
	printForwardingForCall (call, ORDER_PRE, false, true, call->isWrappedAcross());

	out
		<< "\t\t\t</pre>"
		<< std::endl
		<< "\t\t\t<post>"
		<< std::endl;

	//4) ----------- Post -----------
	execOps.clear();
	if (hasPostIntegrity)
	{
		//Integrity input Ops
		printIntegritySourcePieces (call, ORDER_POST, &execOps);

		//Post Integrity
		if (execAnalyses)
		    printExecAnalyses (call, true, ORDER_POST, true);
	}

	//Post Ops
	printSourcePieces (call, false, ORDER_POST, &execOps);

	//Post Analyses
	if (execAnalyses)
	    printExecAnalyses (call, false, ORDER_POST, true);

	//forwarding
	printForwardingForCall (call, ORDER_POST, false, true, call->isWrappedAcross());

	out
		<< "\t\t\t</post>"
		<< std::endl
		<< "\t\t\t<cleanup>"
		<< std::endl;

	//5) ----------- Clean up -----------
	printSourcePieces (call, true, ORDER_PRE, NULL);
	printSourcePieces (call, true, ORDER_POST, NULL);

	out
		<< "\t\t\t</cleanup>"
		<< std::endl
		<< "\t\t</call>"
		<< std::endl;
}

//=============================
// printSourcePieces
//=============================
void Wrapper::printSourcePieces (
		Call *call,
		bool printCleanup,
		CalculationOrder order,
		std::list<std::pair<Operation*, int> > *pExecutedOps)
{
	/*
	 * This map is used to sort all operations to execute according to their
	 * internal call order.
	 * We use the internal map for cases were multiple mappings with an equal
	 * internal order exist.
	 */
	std::map <int, std::map<Mapping*, Operation*> > execOps;
	std::map <int, std::map<Mapping*, Operation*> >::iterator execIter;

	//==Add suitable operation mappings to execOps
	std::map<Call*, CallProperties*> *props = getCallPropertiesForOrder(order);
	if (props->find(call) != props->end())
	{
		CallProperties* prop = (*props)[call];

		std::list<std::pair<Operation*, int > > ops = prop->getMappedOperations();
		std::list<std::pair<Operation*, int > >::iterator opIter;

		for (opIter = ops.begin(); opIter != ops.end(); opIter++)
		{
		    Operation* op = opIter->first;
			int id = opIter->second;

			//was it already executed ?
			if (pExecutedOps)
			{
				std::list<std::pair<Operation*, int> >::iterator execOpIter;
				bool reject=false;

				for (execOpIter = pExecutedOps->begin(); execOpIter != pExecutedOps->end(); execOpIter++)
				{
					if (execOpIter->first == op && execOpIter->second == id)
						reject = true;
				}

				if (reject)
					continue;

				pExecutedOps->push_back(std::make_pair(op, id));
			}

			//Add it to list of operations to execute
			Mapping* m = op->getMappingForCall (call, id);

			if (execOps.find(m->getIntraCallOrder()) != execOps.end())
			{
				execOps[m->getIntraCallOrder()].insert (std::make_pair(m, op));
			}
			else
			{
				std::map<Mapping*, Operation*> temp;
				temp.insert (std::make_pair (m, op));
				execOps.insert (std::make_pair(m->getIntraCallOrder(),temp));
			}
		}
	}

	//==Execute the mappings in execOps
	for (execIter = execOps.begin(); execIter != execOps.end(); execIter++)
	{
		std::map<Mapping*, Operation*>::iterator mappIter;

		for (mappIter = execIter->second.begin(); mappIter != execIter->second.end(); mappIter++)
		{
			Mapping* m = mappIter->first;
			Operation* op = mappIter->second;

			std::string code;

			if (!printCleanup)
				code = op->replaceSourceForMapping (m);
			else
				code = op->replaceCleanupForMapping (m);

			code = SpecificationNode::textToXmlText (code);

			out
			<< "\t\t\t\t<source-piece>"
			<< code
			<< "</source-piece>"
			<< std::endl;
		}
	}
}

//=============================
// printIntegritySourcePieces
//=============================
void Wrapper::printIntegritySourcePieces (
		Call *call,
		CalculationOrder order,
		std::list<std::pair<Operation*, int> > *pExecutedOps)
{
	/*
	 * This map is used to sort all operations to execute according to their
	 * internal call order.
	 * We use the internal map for cases were multiple mappings with an equal
	 * internal order exist.
	 */
	std::map <int, std::map<Mapping*, Operation*> > execOps;
	std::map <int, std::map<Mapping*, Operation*> >::iterator execIter;

	//==Add suitable operation mappings to execOps
	//=> Go over all analyses
	std::list<Analysis*>::iterator analysisIter;
	for (analysisIter = myAnalyses.begin(); analysisIter != myAnalyses.end(); analysisIter++)
	{
		Analysis *analysis = *analysisIter;

		//Only run integrities if it is actually time for them ...
		if (!analysis->getModule()->isLocalIntegrity())
			continue;

		std::list<Mapping*> mappings = analysis->getMappingsForCall(call);
		std::list<Mapping*>::iterator mIter;

		//=> Go over all mappings of integrities to this call
		for (mIter = mappings.begin(); mIter != mappings.end(); mIter++)
		{
			Mapping* m = *mIter;
			if (m->getOrder() != order)
				continue;

			std::vector<Input*> inputs = m->getArgumentInputs();

			//=> Go over all inputs used in a mapping for this call and order
			for (int i = 0; i < inputs.size(); i++)
			{
				Input* input = inputs[i];

				Operation *op;
				int id;

				//Does this input need an op ? If so this is an integrity input!
				if (input->needsOperation(&op, &id))
				{
					//Is the op new ?
					std::list<std::pair<Operation*, int> >::iterator execOpIter;
					bool reject=false;

					for (execOpIter = pExecutedOps->begin(); execOpIter != pExecutedOps->end(); execOpIter++)
					{
						if (execOpIter->first == op && execOpIter->second == id)
							reject = true;
					}

					if (reject)
						continue;

					pExecutedOps->push_back(std::make_pair(op,id));

					//Get the mapping of the op
					Mapping *mOp = op->getMappingForCall(call, id);

					//Is the order reasonsable?
					//Order is correct if it is pre (pre ops can be used for post and pre analyses), or both orders are post
					if (    mOp->getOrder() != ORDER_PRE &&
					        mOp->getOrder() != order)
					    continue;

					//Add it to the list of ops to execute
					if (execOps.find(mOp->getIntraCallOrder()) != execOps.end())
					{
						execOps[mOp->getIntraCallOrder()].insert (std::make_pair(mOp, op));
					}
					else
					{
						std::map<Mapping*, Operation*> temp;
						temp.insert (std::make_pair (mOp, op));
						execOps.insert (std::make_pair(mOp->getIntraCallOrder(),temp));
					}
				}
			}
		}
	}

	//==Execute the mappings in execOps
	for (execIter = execOps.begin(); execIter != execOps.end(); execIter++)
	{
		std::map<Mapping*, Operation*>::iterator mappIter;

		for (mappIter = execIter->second.begin(); mappIter != execIter->second.end(); mappIter++)
		{
			Mapping* m = mappIter->first;
			Operation* op = mappIter->second;

			std::string code;

			code = op->replaceSourceForMapping (m);

			code = SpecificationNode::textToXmlText (code);

			out
			<< "\t\t\t\t<source-piece>"
			<< code
			<< "</source-piece>"
			<< std::endl;
		}
	}
}


/*EOF*/
