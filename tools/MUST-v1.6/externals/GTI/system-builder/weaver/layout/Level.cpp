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
 * @file Level.cpp
 * 		@see gti::weaver::Level
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include <deque>
#include <fstream>
#include <assert.h>
#include <ApiCalls.h>

#include "Level.h"
#include "Wrapper.h"
#include "ReceiveForwarding.h"
#include "ModuleConfig.h"
#include "Verbose.h"
#include "Analyses.h"
#include "Layout.h"

using namespace gti::weaver::layout;

//=============================
// Level
//=============================
Level::Level ( )
: myOrder (-1),
  mySize (0),
  myInList (),
  myOutList (),
  myAnalyses (),
  myAnalysisModules (),
  myPlace (NULL),
  myCallPropertiesPre (),
  myCallPropertiesPost (),
  myReductionForwards (),
  myWrapperSourceName (""),
  myReceivalSourceName (""),
  myOpInputsToFree (),
  myIntraCommunication (NULL),
  myProfiling (false)
{
    if (getenv("GTI_PROFILE") != NULL)
    {
        if (atoi(getenv("GTI_PROFILE")) == 1)
            myProfiling = true;
    }
}

//=============================
// Level
//=============================
Level::Level (int order, uint64_t size, Place *place)
: myOrder (order),
  mySize (size),
  myInList (),
  myOutList (),
  myAnalyses (),
  myAnalysisModules (),
  myPlace (place),
  myCallPropertiesPre (),
  myCallPropertiesPost (),
  myReductionForwards (),
  myWrapperSourceName (""),
  myReceivalSourceName (""),
  myOpInputsToFree (),
  myIntraCommunication (NULL),
  myProfiling (false)
{
    if (getenv("GTI_PROFILE") != NULL)
    {
        if (atoi(getenv("GTI_PROFILE")) == 1)
            myProfiling = true;
    }
}

//=============================
// ~Level
//=============================
Level::~Level ( )
{
	freeArcs (&myInList);
	freeArcs (&myOutList);

	/*Analyses need not be cleared, their memory is
	 *managed by the Analyses singleton.
	 * Same for AnalysisModules.
	 */
	myAnalyses.clear();
	myAnalysisModules.clear ();

	myPlace = NULL;

	//clear all call properties
	std::map<Call*, CallProperties*>::iterator iter;
	for (iter = myCallPropertiesPre.begin(); iter != myCallPropertiesPre.end(); iter++)
	{
		if (iter->second)
			delete iter->second;
	}

	for (iter = myCallPropertiesPost.begin(); iter != myCallPropertiesPost.end(); iter++)
	{
		if (iter->second)
			delete iter->second;
	}

	myCallPropertiesPre.clear ();
	myCallPropertiesPost.clear ();

	myReductionForwards.clear();

	//Free temporary operation inputs
	std::list<OperationInput*>::iterator toFreeIter;
	for (toFreeIter = myOpInputsToFree.begin(); toFreeIter != myOpInputsToFree.end(); toFreeIter++)
	{
		if (*toFreeIter)
			delete (*toFreeIter);
	}
	myOpInputsToFree.clear();

	//Intra communication
	myIntraCommunication = NULL; //memory managed by layout singleton
}

//=============================
// addInArc
//=============================
bool Level::addInArc ( Adjacency * add_object )
{
	for (int i = 0; i < myInList.size(); i++)
	{
		//is such an adjacency already in the list
		if (myInList[i]->getTarget()->getOrder() ==
			add_object->getTarget()->getOrder())
			return false;
	}

	myInList.push_back (add_object);
	return true;
}

//=============================
// getInList
//=============================
std::vector<Adjacency *> Level::getInList ( )
{
	return myInList;
}

//=============================
// addOutArc
//=============================
bool Level::addOutArc ( Adjacency * add_object )
{
	for (int i = 0; i < myOutList.size(); i++)
	{
		//is such an adjacency already in the list
		if (myOutList[i]->getTarget()->getOrder() ==
				add_object->getTarget()->getOrder())
			return false;
	}

	myOutList.push_back (add_object);
	return true;
}

//=============================
// getOutList
//=============================
std::vector<Adjacency*> Level::getOutList ( )
{
	return myOutList;
}

//=============================
// setOrder
//=============================
void Level::setOrder ( int new_var )
{
	myOrder = new_var;
}

//=============================
// getOrder
//=============================
int Level::getOrder ( )
{
	return myOrder;
}

//=============================
// setSize
//=============================
void Level::setSize ( int new_var )
{
	mySize = new_var;
}

//=============================
// getSize
//=============================
int Level::getSize ( )
{
	return mySize;
}

//=============================
// addAnalysis
//=============================
void Level::addAnalysisModule (AnalysisModule* anModule, bool allowReductions)
{
	std::list<AnalysisModule*>::iterator i;

	for (i = myAnalysisModules.begin(); i != myAnalysisModules.end(); i++)
	{
		//already added.
		if (*i == anModule)
			return;
	}

	//add
	if (!allowReductions && !anModule->isReduction())
		myAnalysisModules.push_back(anModule);

	if (allowReductions)
		myAnalysisModules.push_back(anModule);

	//Add analyses of the analysis module
	std::list<Analysis*> analyses = anModule->getAnalyses();
	std::list<Analysis*>::iterator anIter;
	for (anIter = analyses.begin(); anIter != analyses.end(); anIter++)
	{
		myAnalyses.push_back(*anIter);
	}

	//add dependencies
	std::list<AnalysisModule*> dependencies = anModule->getDependencies();

	for (i = dependencies.begin(); i != dependencies.end(); i++)
	{
		//recursive add
		//allowReductions is set to false here, as a dependency of a module must never be a reduction
		//as it couldn't be guaranteed that the dependent reduction would really be placed ...
		addAnalysisModule (*i, false);
	}
}

//=============================
// getAnalyses
//=============================
std::list<Analysis*> Level::getAnalyses (void)
{
	return myAnalyses;
}

//=============================
// getAnalysisModules
//=============================
std::list<AnalysisModule*> Level::getAnalysisModules (void)
{
	return myAnalysisModules;
}

//=============================
// isAnalysisModulePlaced
//=============================
bool Level::isAnalysisModulePlaced (AnalysisModule* module)
{
	std::list<AnalysisModule*>::iterator i;

	for (i = myAnalysisModules.begin(); i != myAnalysisModules.end(); i++)
	{
		if (*i == module)
			return true;
	}

	return false;
}

//=============================
// print
//=============================
std::ostream& Level::print (std::ostream& out) const
{
	out
		<< "level={"
		<< "order=" << myOrder << ", "
		<< "size=" << mySize << ", "
		<< "inArcs={";

	for (int i = 0; i < myInList.size(); i++)
	{
		if (!myInList[i]) continue;

		if (i != 0) out << ", ";

		out << *(myInList[i]);
	}

	out << "}, outArcs={";

	for (int i = 0; i < myOutList.size(); i++)
	{
		if (!myOutList[i]) continue;

		if (i != 0) out << ", ";

		out << *(myOutList[i]);
	}

	out << "}, analyses={";

	std::list<Analysis*>::const_iterator iter;
	for (iter = myAnalyses.begin(); iter != myAnalyses.end(); iter++)
	{
		if (!*iter) continue;

		if (iter != myAnalyses.begin()) out << ", ";

		out << *iter;
	}

	out << "}}";

	return out;
}

//=============================
// getPlace
//=============================
Place* Level::getPlace ()
{
	return myPlace;
}

//=============================
// getReachableNodes
//=============================
std::map<Level*, bool> Level::getReachableNodes (void)
{
	std::map<Level*, bool> reachableNodes;
	std::deque<Level*> toVisit;

	toVisit.push_back (this);
	reachableNodes.insert (std::make_pair(this, true));

	while (!toVisit.empty())
	{
		//pop
		Level* current = toVisit.front();
		toVisit.pop_front();

		//add reachable nodes
		std::vector<Adjacency*> outList = current->getOutList();

		for (int i = 0; i < outList.size(); i++)
		{
			Level* reachable = (outList[i])->getTarget();
			assert (reachable);

			//Already visisted this level ?
			if (reachableNodes.find(reachable) != reachableNodes.end())
				continue;

			//Add to visit and reachable lists
			toVisit.push_back (reachable);
			reachableNodes.insert (std::make_pair(reachable,true));
		}
	}

	return reachableNodes;
}

//=============================
// isAcyclic
//=============================
bool Level::isAcyclic (int *pOutNumReachable)
{
	std::map<Level*, bool> reachableNodes;
	std::map<Level*, int> numOutstandingArcs;
	std::deque<Level*> toVisit;
	int numHandled = 0;

	//1) Find all reachable nodes
	reachableNodes = getReachableNodes ();

	if (pOutNumReachable)
		*pOutNumReachable = reachableNodes.size();

	//2) backward propagation
	//   calculate number of out arcs, and mark sinks
	std::map<Level*, bool>::iterator iter;
	for (iter = reachableNodes.begin(); iter != reachableNodes.end(); iter++)
	{
		int num = iter->first->getOutList().size();

		if (num == 0 &&
			numOutstandingArcs.find(iter->first) == numOutstandingArcs.end())
		{
			toVisit.push_back(iter->first);
		}

		numOutstandingArcs.insert (std::make_pair(iter->first, num));
	}

	while (!toVisit.empty())
	{
		//pop
		Level* current = toVisit.front();
		toVisit.pop_front();
		numHandled++;

		//iterate over in arcs
		std::vector<Adjacency*> inList = current->getInList();

		for (int i = 0; i < inList.size(); i++)
		{
			Level* reachable = (inList[i])->getTarget();

			if (numOutstandingArcs[reachable] == 1)
				toVisit.push_back(reachable);

			numOutstandingArcs[reachable] = numOutstandingArcs[reachable] - 1;
		}
	}

	if (numHandled == reachableNodes.size())
		return true;

	return false;
}

//=============================
// reduceToOneInArc
//=============================
void Level::reduceToOneInArc (void)
{
	int maxOrder = -1;
	Adjacency maxAdjacency;

	//enough arcs
	if (myInList.size() <= 1)
		return;

	//find level with highest order
	for (int i = 0; i < myInList.size(); i++)
	{
		int levelOrder = myInList[i]->getTarget()->getOrder();

		if (levelOrder > maxOrder)
		{
			maxOrder = levelOrder;
			maxAdjacency = *(myInList[i]);
		}
	}

	assert (maxOrder >= 0); //implementation error if not

	//remove all corresponding out arcs for the in arcs
	for (int i = 0; i < myInList.size(); i++)
	{
		int levelOrder = myInList[i]->getTarget()->getOrder();

		if (levelOrder != maxOrder)
		{
			myInList[i]->getTarget()->removeOutArc(this);
		}
	}

	//free all in arcs
	freeArcs (&myInList);

	//add the in arc with highest order
	Adjacency *arc = new Adjacency();
	*arc = maxAdjacency;
	addInArc (arc);
}

//=============================
// removeOutArc
//=============================
void Level::removeOutArc (Level *targetLevel)
{
	for (int i = 0; i < myOutList.size(); i++)
	{
		if (myOutList[i]->getTarget() == targetLevel)
		{
			//free adjacency
			delete myOutList[i];

			//Remove element
			myOutList.erase(myOutList.begin()+i);
			break;
		}
	}
}

//=============================
// freeArcs
//=============================
void Level::freeArcs (std::vector<Adjacency*> *target)
{
	for (int i = 0; i < target->size(); i++)
	{
		if ((*target)[i])
			delete ((*target)[i]);
	}
	target->clear();
}

//=============================
// calculateReductionPlacement
//=============================
GTI_RETURN Level::calculateReductionPlacement (void)
{
	/*
	 * For all levels i:
	       0) Place all reductions accepted by some analysis on i onto i
	           Do not add any ReductionForward mappings
	           (ReductionForward stores for which output channels of i, the reduced record of an event (call + order)
	            may be forwarded)
	       1) Loop over all out channels (leading to place j)
   	   	   	   a)-> Get all reductions accepted at j and descendants (j) -> R
   	   	   	   b)-> For all r in R:
         	 	 	 I)  --> Get list of events E to which r is mapped (call, order)
         	 	 	 II) --> rAcceptable = true
         	 	 	 III)--> For all e in E:
                                ---> Get all analyses run on j and descendants(j) -> A
                                ---> For all a in A:
                                    ----> If a is mapped to e:
                                    ----> If a not accepts r
                                         ------> rAcceptable = false
                                    ----> store e in forwardedToJ
                     IV)--> If rAcceptable
                                ---> Place r on i (if not yet placed)
                                ---> For all events e in forwardedToJ:
                                         -----> Add e,j to ReductionForwards

     * Note:
     *     as adding a reduction to a level may drag in its dependent modules, it is necessary
     *     to start adding reductions at the leaves (i.e. to do a depth first implementation).
     *     In that way these extra modules will already be added when deciding whether a
     *     reduction is suitable or not.
	 */

	//==-1) Recurse into descendants
	for (int i = 0; i < myOutList.size(); i++)
	{
		GTI_RETURN ret;
		Adjacency *adja = myOutList[i];
		Level *target = adja->getTarget();

		ret = target->calculateReductionPlacement();
		if (ret  != GTI_SUCCESS)
			return ret;
	}

	//Only place reductions on levels that are not the application layer, no inter-process reductions are possible on the application layer
	if (getOrder() == 0)
		return GTI_SUCCESS;

	//== 0) Add all reductions accepted by some analysis of this level
	std::list<AnalysisModule*>::iterator iterOwnMods;
	for (iterOwnMods = myAnalysisModules.begin(); iterOwnMods != myAnalysisModules.end(); iterOwnMods++)
	{
		AnalysisModule *mod = *iterOwnMods;

		std::list<AnalysisModule*> supported = mod->getSupportedReductions();
		std::list<AnalysisModule*>::iterator suppIter;

		for (suppIter = supported.begin(); suppIter != supported.end(); suppIter++)
		{
			AnalysisModule *suppModule = *suppIter;

			if (!isAnalysisModulePlaced(suppModule))
				addAnalysisModule (suppModule, true);
		}
	}

	//== 1) Loop over all out channels
	for (int i = 0; i < myOutList.size(); i++)
	{
		Adjacency *adja = myOutList[i];
		Level *target = adja->getTarget();

		//== 1.a) Get all reductions accepted at j and its descendants
		std::list<AnalysisModule*> descendantAnalyses;
		std::list<AnalysisModule*> reductions;
		std::list<AnalysisModule*>::iterator redIter;
		std::deque<Level*> toVisit;
		toVisit.push_back (target); //stack to go over all descendants of level "target"

		while (!toVisit.empty())
		{
			Level *cur = toVisit.front();
			toVisit.pop_front();

			//Add analyses of current level
			std::list<AnalysisModule*> analyses = cur->getAnalysisModules();
			std::list<AnalysisModule*>::iterator anIter;

			for (anIter = analyses.begin(); anIter != analyses.end(); anIter++)
			{
				AnalysisModule* analysis = *anIter;

				//Get supported reductions of this analysis and add them
				std::list<AnalysisModule*> supported = analysis->getSupportedReductions();
				std::list<AnalysisModule*>::iterator suppIter;

				for (suppIter = supported.begin(); suppIter != supported.end(); suppIter++)
					reductions.push_back (*suppIter);

				//Add to overall list analyses on descendants
				descendantAnalyses.push_back(analysis);
			}

			//Add child levels of current level
			std::vector<Adjacency *> currOut = cur->getOutList();

			for (int j = 0; j < currOut.size(); j++)
			{
				toVisit.push_back(currOut[j]->getTarget());
			}
		}//Collect all accepted reductions of target level and all descendants�

		//== 1.b) Loop over all the reductions and determine whether it is applicable
		for (redIter = reductions.begin(); redIter != reductions.end(); redIter++)
		{
			AnalysisModule *reduction = *redIter;
			assert (reduction->isReduction());
			bool reductionWarned = false;

/*
 * The lines below make no sense to me.
 * What they appear to do is:
 * - If some reduction was already placed, it is not checked whether this reduction
 *    is applicable for reducing towards the current target level, but rather
 *    it is just assumed that this works.
 *
 * This makes no sense to me at all, commented out. Should be deleted if this was a sound decision
 */
			/*
			if (isAnalysisModulePlaced(reduction))
			{
				//If this reduction is already placed, add this channel to the list of channels to forward to for the reduction
				std::map<std::pair<Call*, CalculationOrder>, std::list<int> >::iterator redForwardIter;

				for (redForwardIter = myReductionForwards.begin(); redForwardIter != myReductionForwards.end(); redForwardIter++)
				{
					Call *c = redForwardIter->first.first;
					CalculationOrder order = redForwardIter->first.second;

					if (reduction->isMappedTo(c, order))
					{
						redForwardIter->second.push_back(i);
					}
				}

				continue;
			}
			*/

			//== 1.b.II) Set as acceptable
			bool reductionOk = true;

			//== 1.b.I) + 1.b.III) Loop over all events e to which r is mapped
			//Go over all analysis functions of the reduction
			std::list<Analysis*> analyses = reduction->getAnalyses();
			std::list<Analysis*>::iterator anIter;
			std::list<Mapping*> allReductionMappings;

			for (anIter = analyses.begin(); anIter != analyses.end(); anIter++)
			{
				Analysis* analysis = *anIter;

				//Go over all mappings of the current analysis function
				std::vector<Mapping*> mappings = analysis->getCallMappings();

				for (int j = 0; j < mappings.size(); j++)
				{
					Mapping *m = mappings[j];
					allReductionMappings.push_back(m);

					//Loop over all analyses executed on target level and its descendants
					std::list<AnalysisModule*>::iterator descAnIter;
					for (descAnIter = descendantAnalyses.begin(); descAnIter != descendantAnalyses.end(); descAnIter++)
					{
						AnalysisModule* testAnalysis = *descAnIter;

						if (testAnalysis->isMappedTo (m->getApiCall(), m->getOrder()) &&
							!testAnalysis->supportsReduction (reduction) &&
							testAnalysis != reduction)
						{
							//Give some warning/verbose output to help the user to understand why reductions were not mapped
							if (!reductionWarned || (Verbose::getInstance()->getLevel() > 1))
							{
								std::cout
								<< "Info: the reduction named \""
								<< reduction->getName()
								<< "\" in analysis group \""
								<< reduction->getGroup()->getGroupName()
								<< "\" could not be mapped to level "
								<< getOrder()
								<< ". It might have helped to reduce the data amount sent to level "
								<< target->getOrder()
								<< ", but the analysis named \""
								<< testAnalysis->getName()
								<< "\" of group \""
								<< testAnalysis->getGroup()->getGroupName()
								<< "\" running on the target level or one of its descendants does not supports this reduction." << std::endl;
							}

							reductionWarned = true;

							//Reduction can't be used
							reductionOk = false;
							break;
						}
					}
				} //Loop over mappings

				//Abort if reduction is already inapplicable�
				if (!reductionOk)
					break;
			}//Loop over analysis functions of analysis module

			//== 1.b.IV) Check whether reduction is applicable
			if (reductionOk)
			{
				//Add reduction
				if (!isAnalysisModulePlaced(reduction))
					addAnalysisModule (reduction, true);
				VERBOSE(1) << " | -> Info: Reduction named \"" << reduction->getName() << "\" of group \"" << reduction->getGroup()->getGroupName() << "\" on level " << getOrder() << " reduces records forwarded to level " << target->getOrder() << "." << std::endl;

				//Store for which events the reduction needs to be forwarded to what out channels
				std::list<Mapping*>::iterator mapIter;
				for (mapIter = allReductionMappings.begin(); mapIter != allReductionMappings.end(); mapIter++)
				{
					Mapping *m = *mapIter;

					if (myReductionForwards.find(std::make_pair (m->getApiCall(), m->getOrder())) != myReductionForwards.end())
					{
						myReductionForwards[std::make_pair (m->getApiCall(), m->getOrder())].push_back(i);
					}
					else
					{
						std::list<int> channels;
						channels.push_back (i);
						myReductionForwards.insert(std::make_pair(std::make_pair (m->getApiCall(), m->getOrder()), channels));
					}
				}
			}//If reduction is going to be placed

		}//Loop over reductions that might be placed on this level
	} // for all out channels

	//DEBUG
	/*
	std::map<std::pair<Call*, CalculationOrder>, std::list<int> >::iterator redForwardIter;

	for (redForwardIter = myReductionForwards.begin(); redForwardIter != myReductionForwards.end(); redForwardIter++)
	{
		Call *c = redForwardIter->first.first;
		CalculationOrder order = redForwardIter->first.second;
		std::list<int> channels= redForwardIter->second;
		std::list<int>::iterator iIter;

		std::cout << "Level " << getOrder() << " has a reduction forwarding for call " << c->getName() << " to levels: ";
		for (iIter = channels.begin(); iIter != channels.end(); iIter++)
		{
			if (iIter != channels.begin()) std::cout << ", ";
			std::cout << myOutList[*iIter]->getTarget()->getOrder();
		}
		std::cout << std::endl;
	}
	 */
	//DEBUG

	return GTI_SUCCESS;
}

//=============================
// calculateUsedArgs
//=============================
void Level::calculateUsedArgs (void)
{
	std::list<Analysis*>::iterator anIter;

	//DEBUG// std::cout << "Level: " << myOrder << ":" << std::endl;

	//==1) Loop over analyses
	for (anIter = myAnalyses.begin(); anIter != myAnalyses.end(); anIter++)
	{
		Analysis *a = *anIter;
		std::vector<Mapping *> mappings = a->getCallMappings ();

		//Loop over mappings of current analysis
		for (int mI = 0; mI < mappings.size(); mI++)
		{
			Mapping *m = mappings[mI];
			std::vector<Input*> inputs = m->getArgumentInputs ();
			std::list<Input*> inputList;
			Call* c = m->getApiCall();

			//select the right properties (pre or post)
			std::map<Call*, CallProperties*> *propertiesToUse = getCallPropertiesForOrder(m->getOrder());

			//Loop over inputs of Mapping (and put them into a list)
			for (int iI = 0; iI < inputs.size(); iI++)
				inputList.push_back(inputs[iI]);

			//Consistency: Application layer can't get a hold of wrapp-down calls, so it should not try that!
			if (c->isWrappedDown() && getOrder() == 0)
			{
			    std::cerr
			        << " | -> Warning: the application layer (layer 0) uses the analysis module "
			        << a->getName()
			        << ", which is mapped to the wrapp-down call "
			        << c->getName()
			        << "; however, the application layer can never receive wrapp-down calls, so the module may not operate as intended." << std::endl;
			}

			//register call and input in used args
			if (propertiesToUse->find(c) == propertiesToUse->end())
			{
				//new properties
				bool isOnApplication=false;
				if (myOrder == 0)
					isOnApplication=true;

				CallProperties *p = new CallProperties (c, isOnApplication);
				p->addUsedArgs(inputList);
				propertiesToUse->insert (std::make_pair(c, p));
			}
			else
			{
				//existing properties
				(*propertiesToUse)[c]->addUsedArgs(inputList);
			}
		}//for mappings
	}//for analyses

	//==2) Add properties for all finalizer calls
	//As pre properties, the properties themselves look at the call and
	//will answer their needWrapper calls and such according to the
	//rules needed for finalizer calls
	std::list<Call*> finCalls = ApiCalls::getInstance()->getFinalizerCalls ();
	std::list<Call*>::iterator finIter;

	for (finIter=finCalls.begin(); finIter != finCalls.end(); finIter++)
	{
		Call* c = *finIter;

		//add property for finalizer call c if not yet listed
		if (myCallPropertiesPre.find(c) == myCallPropertiesPre.end())
		{
			//new properties
			bool isOnApplication=false;
			if (myOrder == 0)
				isOnApplication=true;

			CallProperties *p = new CallProperties (c, isOnApplication);
			myCallPropertiesPre.insert (std::make_pair(c, p));
		}
	}
}

//=============================
// calculateArgsToReceive
//=============================
void Level::calculateArgsToReceive (void)
{
	std::map<Level*, bool> reachableNodes;
	std::map<Level*, int> numOutstandingArcs;
	std::deque<Level*> toVisit;

	//1) Find all reachable nodes
	reachableNodes = getReachableNodes ();

	//2) backward propagation
	//   calculate number of out arcs, and mark sinks
	//a) Find sinks and put them on the stack
	std::map<Level*, bool>::iterator iter;
	for (iter = reachableNodes.begin(); iter != reachableNodes.end(); iter++)
	{
		int num = iter->first->getOutList().size();

		if (num == 0 &&	numOutstandingArcs.find(iter->first) == numOutstandingArcs.end())
			toVisit.push_back(iter->first);

		numOutstandingArcs.insert (std::make_pair(iter->first, num));
	}

	//b) loop over sink stack until all done
	while (!toVisit.empty())
	{
		//pop
		Level* current = toVisit.front();
		toVisit.pop_front();

		//iterate over in arcs
		std::vector<Adjacency*> inList = current->getInList();

		for (int i = 0; i < inList.size(); i++)
		{
			Level* reachable = (inList[i])->getTarget();

			if (numOutstandingArcs[reachable] == 1)
				toVisit.push_back(reachable);

			numOutstandingArcs[reachable] = numOutstandingArcs[reachable] - 1;

			//Give the ancestor all our required inputs
			//DEBUG// std::cout << "Propagate from " << current->getOrder() << " to " << reachable->getOrder() << ":" << std::endl;
			reachable->addArgsFromDescendant (ORDER_PRE, current->myCallPropertiesPre, false);
			reachable->addArgsFromDescendant (ORDER_POST, current->myCallPropertiesPost, false);
		}
	}
}

//=============================
// calculateArgsToReceiveReverse
//=============================
void Level::calculateArgsToReceiveReverse (void)
{
    std::deque<Level*> toVisit;
    toVisit.push_back(this);

    while (!toVisit.empty())
    {
        //pop
        Level* current = toVisit.front();
        toVisit.pop_front();

        //iterate over out arcs
        std::vector<Adjacency*> outList = current->getOutList();

        for (int i = 0; i < outList.size(); i++)
        {
            Level* reachable = (outList[i])->getTarget();
            toVisit.push_back(reachable);

            //Give the descendant all our required wrapp-down inputs
            reachable->addArgsFromDescendant (ORDER_PRE, current->myCallPropertiesPre, true);
            reachable->addArgsFromDescendant (ORDER_POST, current->myCallPropertiesPost, true);
        }
    }
}

//=============================
// addArgsFromDescendant
//=============================
void Level::addArgsFromDescendant (
        CalculationOrder order,
        std::map<Call*, CallProperties*> properties,
        bool forWrappDown)
{
	std::map<Call*, CallProperties*>::iterator iter;

	std::map<Call*, CallProperties*> *targetProperties = getCallPropertiesForOrder(order);

	for (iter = properties.begin(); iter != properties.end(); iter++)
	{
		Call *c = iter->first;
		CallProperties *p = iter->second;

		//Skipp or not skip wrapp-down calls depending on forWrappDown setting
		if (forWrappDown != c->isWrappedDown())
		    continue;

		/**
		 * Todo I think in both cases we should never propagate this for intralayer calls,
		 * but I gues it doesn't hurts since we generate no forwarding for them ...
		 */

		//Copy
		if (targetProperties->find(c) == targetProperties->end())
		{
			//no properties for this call yet
			bool isOnApplication=false;
			if (myOrder == 0)
				isOnApplication=true;
			CallProperties *newP = new CallProperties (c, isOnApplication);
			newP->addArgsToReceive (p->getArgsToReceive());
			targetProperties->insert (std::make_pair(c, newP));
		}
		else
		{
			//add to existing properties
			(*targetProperties)[c]->addArgsToReceive(p->getArgsToReceive());
		}
	}
}

//=============================
// calculateUIds
//=============================
void Level::calculateUIds (void)
{
	std::map<Call*, CallProperties*>::iterator propIter;
	std::deque<Level*> toVisit;

	//Set initial UId's
	for (propIter = myCallPropertiesPre.begin(); propIter != myCallPropertiesPre.end(); propIter++)
		propIter->second->setInRecordUniqueId(CallProperties::getNextUniqueId());
	for (propIter = myCallPropertiesPost.begin(); propIter != myCallPropertiesPost.end(); propIter++)
		propIter->second->setInRecordUniqueId(CallProperties::getNextUniqueId());

	//Add this (as root) to stack
	toVisit.push_back (this);

	//walk over level graph (its a tree)
	while (!toVisit.empty())
	{
		//pop and get arcs
		Level *l = toVisit.front();
		std::vector<Adjacency*> outArcs = l->getOutList ();

		toVisit.pop_front();

		//walk over descendants
		//Distribute initial uids
		for (int arcI = 0; arcI < outArcs.size(); arcI++)
		{
			Level *descendant = outArcs[arcI]->getTarget ();

			//loop over the two orders (pre,post)
			for (int iOrder = 0; iOrder < 2; iOrder++)
			{
				CalculationOrder order = ORDER_PRE;
				if (iOrder == 1)
					order = ORDER_POST;

				std::map<Call*, CallProperties*> *descendantProps = descendant->getCallPropertiesForOrder(order);
				std::map<Call*, CallProperties*> *currentProps = l->getCallPropertiesForOrder(order);

				//loop over all calls for which we have properties in the descendant
				std::map<Call*, CallProperties*>::iterator propIter;
				for (propIter = descendantProps->begin(); propIter != descendantProps->end(); propIter++)
				{
					Call *c = propIter->first;
					CallProperties *p = propIter->second;

					//Skip wrap-down calls, they have a reversed uid distribution!
					if (c->isWrappedDown())
					    continue;

					//If propagation took place the current level must have all the inputs for the descendant,
					//and thus also have a property entry for the call
					assert (currentProps->find(c) != currentProps->end());

					//Decide the uid for the descendant and the call in question
					if (p->receiveListEqual ((*currentProps)[c]->getArgsToReceive()))
					{
						//descendant takes all the records of this level -> same record -> same uid
						p->setInRecordUniqueId((*currentProps)[c]->getInRecordUniqueId());
					}
					else
					{
						//descendant takes different input record than this level -> different record -> different uid
						p->setInRecordUniqueId(CallProperties::getNextUniqueId());
					}
				}//for properties of descendant
			}

			//Add descendant to stack (its a tree so no further in arcs to the descendant)
			toVisit.push_back (descendant);
		}//for descendants of current

		//Check whether two descendants got different uid's while
		//receiving the same record, if so use an equal uid
		for (int arcI = 0; arcI < outArcs.size(); arcI++)
		{
			for (int higherArcI = arcI+1; higherArcI < outArcs.size(); higherArcI++)
			{
				Level *descendant1 = outArcs[arcI]->getTarget ();
				Level *descendant2 = outArcs[higherArcI]->getTarget ();

				//loop over the two orders (pre,post)
				for (int iOrder = 0; iOrder < 2; iOrder++)
				{
					CalculationOrder order = ORDER_PRE;
					if (iOrder == 1)
						order = ORDER_POST;

					std::map<Call*, CallProperties*> *desc1Props = descendant1->getCallPropertiesForOrder(order);
					std::map<Call*, CallProperties*> *desc2Props = descendant2->getCallPropertiesForOrder(order);

					std::map<Call*, CallProperties*>::iterator propIter;
					for (propIter = desc1Props->begin(); propIter != desc1Props->end(); propIter++)
					{
						//has the other descendant a property for this call ?
						if (desc2Props->find(propIter->first) == desc2Props->end())
							continue;

						//get the two properties and compare them
						CallProperties *p1 = propIter->second;
						CallProperties *p2 = (*desc2Props)[propIter->first];

						if (p1->receiveListEqual(p2->getArgsToReceive()))
						{
							//both descendants have the same args to receive
							//for this call and order
							//=> use an equal uid
							p2->setInRecordUniqueId(p1->getInRecordUniqueId());
						}
					}
				}
			}
		}
	}//while levels on stack
}

//=============================
// calculateUIdsReverse
//=============================
void Level::calculateUIdsReverse (void)
{
    std::map<Level*, bool> reachableNodes;
    std::map<Level*, int> numOutstandingArcs;
    std::deque<Level*> toVisit;
    std::map<Call*, CallProperties*>::iterator propIter;

    //1) Find all reachable nodes
    reachableNodes = getReachableNodes ();

    //2) backward propagation
    //   calculate number of out arcs, and mark sinks
    //a) Find sinks and put them on the stack
    std::map<Level*, bool>::iterator iter;
    for (iter = reachableNodes.begin(); iter != reachableNodes.end(); iter++)
    {
        Level *curr = iter->first;
        int num = curr->getOutList().size();

        if (num == 0 && numOutstandingArcs.find(curr) == numOutstandingArcs.end())
        {
            toVisit.push_back(curr);

            //Set initial UId's
            for (propIter = curr->myCallPropertiesPre.begin(); propIter != curr->myCallPropertiesPre.end(); propIter++)
            {
                if (!propIter->first->isWrappedDown()) continue;
                propIter->second->setInRecordUniqueId(CallProperties::getNextUniqueId());
            }
            for (propIter = iter->first->myCallPropertiesPost.begin(); propIter != iter->first->myCallPropertiesPost.end(); propIter++)
            {
                if (!propIter->first->isWrappedDown()) continue;
                propIter->second->setInRecordUniqueId(CallProperties::getNextUniqueId());
            }
        }

        numOutstandingArcs.insert (std::make_pair(iter->first, num));
    }

    //b) loop over sink stack until all levels are processed
    while (!toVisit.empty())
    {
        //pop and get arcs
        Level *l = toVisit.front();
        toVisit.pop_front();

        std::vector<Adjacency*> inArcs = l->getInList ();

        if (inArcs.size() == 0)
            continue;

        Level *ancestor = inArcs[0]->getTarget ();

        //loop over the two orders (pre,post)
        for (int iOrder = 0; iOrder < 2; iOrder++)
        {
            CalculationOrder order = ORDER_PRE;
            if (iOrder == 1)
                order = ORDER_POST;

            std::map<Call*, CallProperties*> *ancestorProps = ancestor->getCallPropertiesForOrder(order);
            std::map<Call*, CallProperties*> *currentProps = l->getCallPropertiesForOrder(order);

            //loop over all calls for which we have properties in the ancestor
            for (propIter = ancestorProps->begin(); propIter != ancestorProps->end(); propIter++)
            {
                Call *c = propIter->first;
                CallProperties *p = propIter->second;

                //Skip non wrap-down calls, they are handled by the regular uid propagation
                if (!c->isWrappedDown())
                    continue;

                //If propagation took place the current level must have all the inputs for the descendant,
                //and thus also have a property entry for the call
                assert (currentProps->find(c) != currentProps->end());

                //Decide the uid for the ancestor and the call in question
                if (p->receiveListEqual ((*currentProps)[c]->getArgsToReceive()))
                {
                    //ancestor takes all the records of this level -> same record -> same uid
                    p->setInRecordUniqueId((*currentProps)[c]->getInRecordUniqueId());
                }
                else
                {
                    //ancestor takes different input record than this level -> different record -> different uid
                    p->setInRecordUniqueId(CallProperties::getNextUniqueId());
                }
            }//for properties of ancestor
        }

        //Add ancestor to stack (its a tree so no further in arcs to the ancestor)
        numOutstandingArcs[ancestor]--;
        if (numOutstandingArcs[ancestor] == 0)
            toVisit.push_back (ancestor);
    }//while levels on stack
}

//=============================
// getCallPropertiesForOrder
//=============================
std::map<Call*, CallProperties*>* Level::getCallPropertiesForOrder (CalculationOrder order)
{
	if (order == ORDER_PRE)
		return &myCallPropertiesPre;

	return &myCallPropertiesPost;
}

//=============================
// calculateNeededOperations
//=============================
void Level::calculateNeededOperations (void)
{
	CalculationOrder orders[] = {ORDER_PRE, ORDER_POST};
	const int numOrders = 2;

	//for orders (Pre, Post)
	for (int iOrder = 0; iOrder < numOrders; iOrder++)
	{
		CalculationOrder order = orders[iOrder];

		std::map<Call*, CallProperties*> *targetProps = getCallPropertiesForOrder (order);
		std::map<Call*, CallProperties*>::iterator propIter;

		//For properties in current order
		for (propIter = targetProps->begin(); propIter != targetProps->end(); propIter++)
		{
			Call *call = propIter->first;
			CallProperties *prop = propIter->second;

			//we only need properties for calls with wrapper
			if (!prop->needsWrapper())
				continue;

			//For inputs in property
			std::list<Input*> inputs = prop->getArgsToReceive();
			std::list<Input*>::iterator inputIter;
			for (inputIter = inputs.begin(); inputIter != inputs.end(); inputIter++)
			{
				Input* input = *inputIter;
				Operation* op;
				int id;

				if (!input->needsOperation(&op, &id))
					continue;

				//input needs an operation, check the order of its mapping, add if correct order
				Mapping* m = op->getMappingForCall (call, id);

				assert (m); //must exist, otherwise internal error (usage of non mapped operation should be detected during loading)

				//Order is correct if it is pre (pre ops can be used for post and pre analyses), or both orders are post
				if (m->getOrder() != ORDER_PRE &&
					m->getOrder() != order)
					continue;

				if (m->getOrder() == order)
				{
					prop->addOperationToExecute (op, id);
				}
				else
				{
					//Need to find the right pre order property, this is a post order property
					assert (m->getOrder() == ORDER_PRE);
					std::map<Call*, CallProperties*> *preProps = getCallPropertiesForOrder(ORDER_PRE);
					std::map<Call*, CallProperties*>::iterator prePropsPos = preProps->find(call);

					if (prePropsPos != preProps->end())
					{
						//existing property
						prePropsPos->second->addOperationToExecute(op, id);
					}
					else
					{
						//new property
						bool isOnApplication=false;
						if (myOrder == 0)
							isOnApplication=true;

						CallProperties *p = new CallProperties (call, isOnApplication);
						p->addOperationToExecute(op, id);
						preProps->insert (std::make_pair(call, p));
					}
				}
			}//for inputs
		}//for properties
	}//for orders
}

//=============================
// printCallProperties
//=============================
std::ostream& Level::printCallProperties (std::ostream& out)
{
	std::map<Call*, CallProperties*>::iterator i;

	out << std::endl << "========== Property print Level " << myOrder << " ==========" << std::endl;
	out << "=====Pre:" << std::endl;

	for (i = myCallPropertiesPre.begin(); i != myCallPropertiesPre.end(); i++)
	{
		out << *(i->second) << std::endl;
	}

	out << "=====Post:" << std::endl;

	for (i = myCallPropertiesPost.begin(); i != myCallPropertiesPost.end(); i++)
	{
		out << *(i->second) << std::endl;
	}

	return out;
}

//=============================
// generateWrappGenInput
//=============================
void Level::generateWrappGenInput (
		std::string fileName,
		std::string sourceFileName,
		std::string headerFileName,
		std::string logFileName)
{
	//store the name of the source file for the wrapper
	myWrapperSourceName = sourceFileName;

	//generate
	generation::Wrapper (this, fileName, sourceFileName, headerFileName, logFileName);
}

//=============================
// generateReceivalInput
//=============================
void Level::generateReceivalInput (
		std::string fileName,
		std::string sourceFileName,
		std::string headerFileName,
		std::string logFileName)
{
	//Store the name of the source file for receival
	myReceivalSourceName = sourceFileName;

	//generate
	generation::ReceiveForwarding (this, fileName, sourceFileName, headerFileName, logFileName);
}

//=============================
// generateModuleConfigurationInput
//=============================
void Level::generateModuleConfigurationInput (std::string fileName)
{
	generation::ModuleConfig (this, fileName);
}

//=============================
// addChannelIdData
//=============================
GTI_RETURN Level::addChannelIdData ()
{
	int num64s;
	int bitsPerSubId;
	CalculationOrder orders[2] = {ORDER_PRE, ORDER_POST};
	Operation* channOp = Analyses::getInstance()->findOperation("gtiChannelIdOp","GTI_Internal");
	Operation* channOpStrided = Analyses::getInstance()->findOperation("gtiChannelIdOpStrided","GTI_Internal");
	Operation* opToUse = NULL;
	Layout::getInstance()->getChannelIdInfo(&num64s, &bitsPerSubId);

	if (!channOp || !channOpStrided)
	    return GTI_ERROR;

	for (int i = 0; i < 2; i++)
	{
		CalculationOrder order = orders[i];

		std::map<Call*, CallProperties*> *props = getCallPropertiesForOrder (orders[i]);
		std::map<Call*, CallProperties*>::iterator pIter;

		for (pIter = props->begin(); pIter != props->end(); pIter++)
		{
			Call *c = pIter->first;
			CallProperties *p = pIter->second;

			//Intra layer calls and wrapp-down calls never need channel-ids
			if (c->isWrappedAcross() || c->isWrappedDown())
			    continue;

			//Only add to interesting properties
			if (p->needsReceival() || p->needsWrapper())
			{
				std::list<Input*> inputsToAdd;

				for (int n = 0; n < num64s; n++)
				{
					int mappingID = n;
					if (order == ORDER_POST)
						mappingID += 1000;

					opToUse = channOp;
					if (n == num64s - 1)
					    opToUse = channOpStrided;

					OperationInput *input = new OperationInput (opToUse, mappingID);
					inputsToAdd.push_back(input);
					myOpInputsToFree.push_back(input);

					//Add operation to properties if needed
					if (p->needsWrapper())
					    p->addOperationToExecute(opToUse, mappingID);

					//Add mapping to operation if needed
					if (!opToUse->hasMappingForCall (c->getName(), c->getGroup()->getApiName(), mappingID, order))
					    opToUse->addCallMapping(new Mapping (c, order, mappingID, -1));
				}

				//Add input to used args in properties
				p->addUsedArgs(inputsToAdd);
			} //property interesting
		} //for properties in curr order
	}//for orders

	return GTI_SUCCESS;
}

//=============================
// hasIntraCommunication
//=============================
bool Level::hasIntraCommunication (void)
{
    if (myIntraCommunication)
        return true;
    return false;
}

//=============================
// setIntraCommunication
//=============================
GTI_RETURN Level::setIntraCommunication (Communication* intraComm)
{
    if (myIntraCommunication)
    {
        std::cerr<< "Error: this Level already has a inta level communication." << std::endl;
        return GTI_ERROR;
    }

    myIntraCommunication = intraComm;
    return GTI_SUCCESS;
}

//=============================
// checkWrapAcrossUsage
//=============================
GTI_RETURN Level::checkWrapAcrossUsage (void)
{
    //If we have a intra communication, everything is perfect
    if (myIntraCommunication != NULL)
        return GTI_SUCCESS;

    //If we have no intra communication no module on this level must create any wrapp across function
    std::list<AnalysisModule*>::iterator iter;

    for (iter = myAnalysisModules.begin(); iter != myAnalysisModules.end(); iter++)
    {
        AnalysisModule *mod = *iter;

        std::list<Call*> createdBy = mod->getCallsCreatedByModule();
        std::list<Call*>::iterator cIter;

        for (cIter = createdBy.begin(); cIter != createdBy.end(); cIter++)
        {
            Call *c = *cIter;

            if (c->isWrappedAcross())
            {
                //We hanlde this as warning, modules can test for the availability of the wrap across function
                std::cerr << "|->Warning: the module " << mod->getModuleName() << " is mapped to layer " << getOrder() << " and can create a wrap-across call named " << c->getName() << "; However, this layer has no intra communication which limits the functionality of this module, you may want to move the module to a different layer or add an intra communication to this level." << std::endl;
                //return GTI_ERROR;
            }
        }
    }

    return GTI_SUCCESS;
}


//=============================
// checkWrapAcrossUsage
//=============================
GTI_RETURN Level::markWrapAcrossCallsToWrap (void)
{
    std::map<Call*, CallProperties*> *currProperties;
    CalculationOrder orders[2] = {ORDER_PRE, ORDER_POST};

    for (int i = 0; i < 2; i++)
    {
        CalculationOrder order = orders[i];
        currProperties = getCallPropertiesForOrder(order);

        std::map<Call*, CallProperties*>::iterator iter;
        for (iter = currProperties->begin(); iter != currProperties->end(); iter++)
        {
            CallProperties* properties = iter->second;
            Call* call = iter->first;
            std::list<AnalysisModule*>::iterator modIter;

            for (modIter = myAnalysisModules.begin(); modIter != myAnalysisModules.end(); modIter++)
            {
                AnalysisModule* mod = *modIter;
                std::list<Call*> created = mod->getCallsCreatedByModule();
                std::list<Call*>::iterator createIter;

                for (createIter = created.begin(); createIter != created.end(); createIter++)
                {
                    Call* created = *createIter;

                    if (created == call)
                    {
                        //HIT we have a module that creates the call for which we have a property
                        //=> we need to wrap it!
                        properties->setWrapAcrossCallAsCreatedOnLevel ();
                    }
                }
            }
        }
    }

    return GTI_SUCCESS;
}

//=============================
// addAutomagicModules
//=============================
GTI_RETURN Level::addAutomagicModules (void)
{
    // == 1) Get all analyses and fish for the analyses that are from automagic modules
    std::list<Analysis*> automagicAnalyses, allAnalyses;
    allAnalyses = Analyses::getInstance()->getAnalyses("");
    std::list<Analysis*>::iterator anIter;
    for (anIter = allAnalyses.begin(); anIter != allAnalyses.end(); anIter++)
    {
        Analysis* an = *anIter;
        if (!an->getModule()) continue;
        if (an->getModule()->isAddedAutomagically())
            automagicAnalyses.push_back(an);
    }

    // == 2)
    // - Iterate over all our properties (p) for which we would currently create a wrapper or a receival
    // -- Iterate over all automagic analyses
    // --- See if the automagic analysis has a mapping for this property
    // --- If so, check whether we have all of the inputs for the mapping
    // --- If so too, add the module to the layer
    //
    // IMPORTANT: here we only add the module, we check in GenerationBase for each individual mapping whether we
    //                      really got all inputs, and if so we actually add individual analyses of the modules we add here.
    CalculationOrder orders[2] = {ORDER_PRE, ORDER_POST};
    for (int i = 0; i < 2; i++)
    {
        CalculationOrder order = orders[i];
        std::map<Call*, CallProperties*>* currProperties = getCallPropertiesForOrder(order);

        std::map<Call*, CallProperties*>::iterator iter;
        for (iter = currProperties->begin(); iter != currProperties->end(); iter++)
        {
            CallProperties* properties = iter->second;
            Call* call = iter->first;

            for (anIter = automagicAnalyses.begin(); anIter != automagicAnalyses.end(); anIter++)
            {
                Analysis* an = *anIter;
                std::list<Mapping*> mappings = an->getMappingsForCall (call);
                std::list<Mapping*>::iterator mapIter;

                for (mapIter = mappings.begin(); mapIter != mappings.end(); mapIter++)
                {
                    Mapping* m = *mapIter;

                    //Does the order fits?
                    if (m->getOrder() != orders[i])
                        continue;

                    //Ok, so lets look at the inputs
                    std::vector<Input*> mInputs = m->getArgumentInputs();
                    std::list<Input*> mInputsList (mInputs.begin(), mInputs.end());

                    //Do we wrap this at all? (We only apply automagic in wrappers per definition)
                    //Do we have all the inputs we would need for the analysis?
                    if (properties->needsWrapper() && properties->usedOrReceiveArgsContain (mInputsList))
                    {
                        //Bingo, we got all we need for this particular automagic analysis
                        this->addAnalysisModule(an->getModule(), false);
                        //We can abort for the this analysis here, we added the module, nothing else we could do here
                        break;
                    }
                } //for mappings to call (of property) of automagic analyses
            }// for automagic analyses
        } //for properties
    } //for pre/post

    return GTI_SUCCESS;
}

/*EOF*/
