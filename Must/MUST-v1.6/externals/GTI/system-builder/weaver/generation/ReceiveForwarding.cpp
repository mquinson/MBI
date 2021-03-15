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
 * @file ReceiveForwarding.cpp
 * 		@see gti::weaver::generation::ReceiveForwarding
 *
 * @author Tobias Hilbrich
 * @date 13.08.2010
 */

#include <fstream>
#include <assert.h>

#include "ReceiveForwarding.h"
#include "ApiCalls.h"
#include "Layout.h"
#include "Analyses.h"

using namespace gti::weaver::generation;
using namespace gti::weaver::layout;
using namespace gti::weaver::analyses;

//=============================
// ReceiveForwarding
//=============================
ReceiveForwarding::ReceiveForwarding (
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
		<< "<receival-specification>" << std::endl;

	if (!writeSettings(sourceFileName, headerFileName, logFileName))
		return;

	//---------------------------------
	// Write information on channel id
	//---------------------------------
	uint64_t from=0;

	//determine distance to layer 0
	Level *curr = this;
	while (curr->getOrder() != 0)
	{
		std::vector<Adjacency*> a = curr->getInList();
		assert (a.size() != 0);
		curr = a[0]->getTarget();
		from ++;
	}
	from--; //We count distance of the level we receive from to layer 0

	int num64, bitsPerChannel;
	Layout::getInstance()->getChannelIdInfo(&num64,&bitsPerChannel);
	Operation* chanOp = Analyses::getInstance()->findOperation("gtiChannelIdOp","GTI_Internal");
	assert (chanOp); //needs to be present ...
	std::string argumentBase = chanOp->getResultVarName(0);
	argumentBase.resize(argumentBase.length()-1); //TODO uses the implicit knowledge that the mapping id is the last number !

	out
		<< "<channel-id "
		<< "fromLevel=\"" << from << "\" "
		<< "numLevels=\"" << Layout::getInstance()->getNumLevels() << "\" "
		<< "num64s=\"" << num64 << "\" "
		<< "bitsPerChannel=\"" << bitsPerChannel << "\" "
		<< "idArgumentBaseName=\"" << argumentBase << "\" "
		<< "startIndexPre=\"0\" "
		<< "startIndexPost=\"1000\" "
		<< "></channel-id>" << std::endl;

	//---------------------------------
	//create a list of all headers and add it to the XML
	//---------------------------------
	if (!calculateHeaders (false))
		return;
	if (!printHeaders ())
		return;

	//---------------------------------
	//create list of all communication modules and add to XML
	//---------------------------------
	if (!calculateComms())
		return;
	if (!printComms(false))
		return;

	//---------------------------------
	//create list of analysis modules and add it to XML
	//---------------------------------
	if (!calculateAnalyses())
		return;
	if (!printAnalyses())
		return;

	//---------------------------------
	//write out information on each received record
	//---------------------------------
	//list of calls that need to be wrapped
	out << "\t<receivals>" << std::endl;

	CalculationOrder orders[] = {ORDER_PRE, ORDER_POST};
	const int num_orders=2;
	for (int i = 0; i < num_orders; i++)
	{
		std::map<Call*, CallProperties*>* props = getCallPropertiesForOrder (orders[i]);
		std::map<Call*, CallProperties*>::iterator propIter;

		for (propIter = props->begin(); propIter != props->end(); propIter++)
		{
			CallProperties* prop = propIter->second;
			if (prop->needsReceival())
				printReceival (propIter->first, orders[i]);
		}
	}

	out
		<< "\t</receivals>" << std::endl
		<< "</receival-specification>" << std::endl;
}

//=============================
// ~ReceiveForwarding
//=============================
ReceiveForwarding::~ReceiveForwarding (void)
{

}

//=============================
// ~printReceival
//=============================
bool ReceiveForwarding::printReceival (Call *call, CalculationOrder order)
{
	//Initial entries
	out
		<< "\t\t<receival";

	if (call->isFinalizer() && order == ORDER_PRE)
		out << " is-finalizer=\"yes\"";

	if (call->isWrappedAcross())
	    out << " is-wrap-across=\"yes\"";

	if (call->isWrappedDown())
	    out << " is-wrap-down=\"yes\"";

	if (call->isNotifyFinalize())
	    out << " notify-finalize=\"yes\"";

	if (call->isOutOfOrder())
	    out << " out-of-order=\"yes\"";

	out
		<< ">"
		<< std::endl
		<< "\t\t\t<call-name>" << call->getName() << "</call-name>" << std::endl;

	if (order == ORDER_PRE)
		out << "\t\t\t<order>pre</order>" << std::endl;
	else
		out << "\t\t\t<order>post</order>" << std::endl;

	//Print incoming record
	std::map<Call*, CallProperties*> *props = getCallPropertiesForOrder(order);
	CallProperties *ownProp = (*props)[call];
	if (!ownProp) return false;

	printRecord (call, ownProp->getInRecordUniqueId(), ownProp->getArgsToReceive());

	//==print actions
	//exec analyses
	out << "\t\t\t<actions>" << std::endl;
	printExecAnalyses (call, false, order, false);

	//forwarding
	if (!call->isWrappedAcross())
	{
	    //For non wrap-across calls normal forwarding
	    printForwardingForCall (call, order, false, false, false); //Tobias, Dec 09, 2010: set exclude to false, simplifies implementation of receival generator
	}
	else
	{
	    //No forwarding for wrapp-across calls currently!
	    out << "<forwarding><records></records><forwards></forwards></forwarding>" << std::endl;
	}

	//print trailer
	out
		<< "\t\t\t</actions>" << std::endl
		<< "\t\t</receival>" << std::endl;

	return true;
}

/*EOF*/
