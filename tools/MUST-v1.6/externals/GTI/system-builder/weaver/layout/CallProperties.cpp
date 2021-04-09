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
 * @file CallProperties.cpp
 * 		@see gti::weaver::CallProperties
 *
 * @author Tobias Hilbrich
 * @date 05.08.2010
 */

#include "CallProperties.h"

using namespace gti::weaver::layout;

long long CallProperties::ourNextUniqueId = 0;

//=============================
// CallProperties
//=============================
CallProperties::CallProperties (void)
 : myCall (NULL),
   myUsedArgs (),
   myArgsToReceive (),
   myInRecordUniqueId (-1),
   myIsOnApplication (false),
   myOpsToExecute (),
   myInformationRequired (false),
   myWrapAcrossIsCreatedOnLevel (false)
{

}

//=============================
// CallProperties
//=============================
CallProperties::CallProperties (Call* call, bool isOnApplication)
 : myCall (call),
   myUsedArgs (),
   myArgsToReceive (),
   myInRecordUniqueId (-1),
   myIsOnApplication (isOnApplication),
   myOpsToExecute (),
   myInformationRequired (false)
{

}

//=============================
// ~CallProperties
//=============================
CallProperties::~CallProperties (void)
{
	//Memory of the arguments and the calls is
	//managed by their repsective singletons
	myUsedArgs.clear();
	myArgsToReceive.clear();
	myOpsToExecute.clear();
	myCall = NULL;
}

//=============================
// addUsedArgs
//=============================
void CallProperties::addUsedArgs (std::list<Input*> args)
{
	myInformationRequired = true;

	//Adds to both lists
	addArgs (args, &myUsedArgs);
	addArgs (args, &myArgsToReceive);
}

//=============================
// addArgsToReceive
//=============================
void CallProperties::addArgsToReceive (std::list<Input*> args)
{
	myInformationRequired = true;

	addArgs (args, &myArgsToReceive);
}

//=============================
// addArgs
//=============================
void CallProperties::addArgs (std::list<Input*> args, std::list<Input*> *target)
{
	std::list<Input*>::iterator i,j;

	//TODO: performance inefficiency
	for (i = args.begin(); i != args.end(); i++)
	{
		//already in that list ?
		for (j = target->begin(); j != target->end(); j++)
		{
			if ((*i)->getName() == (*j)->getName())
				break;
		}

		if (j == target->end())
		{
			//not yet in list->add
			target->push_back (*i);

			//DEBUG|VERBOSE
			//DEBUG// std::cout << "ADDED INPUT: " << (*i)->getName() << " (call: " << myCall->getName() << ")" << std::endl;
		}
	}
}

//=============================
// getUsedArgs
//=============================
std::list<Input*> CallProperties::getUsedArgs (void)
{
	return myUsedArgs;
}

//=============================
// getArgsToReceive
//=============================
std::list<Input*> CallProperties::getArgsToReceive (void)
{
	return myArgsToReceive;
}

//=============================
// getInRecordUniqueId
//=============================
long long CallProperties::getInRecordUniqueId (void)
{
	return myInRecordUniqueId;
}

//=============================
// setInRecordUniqueId
//=============================
void CallProperties::setInRecordUniqueId (long long uid)
{
	myInRecordUniqueId = uid;
}

//=============================
// needsWrapper
//=============================
bool CallProperties::needsWrapper (void) const
{
	/**
	 * Important:
	 * It is necessary to use the arguments to receive here!
	 * Even if this level does not uses the arguments of the
	 * call (i.e. myUsedArgs is empty), wrapping the call
	 * will generate information that is consumed by other
	 * levels if myArgsToReceive is not empty. Thus, the
	 * information needs to be collected for forwarding in
	 * that case.
	 */
	if (myInformationRequired)
	{
		if (myIsOnApplication || myCall->isWrappedEverywhere() || myCall->isWrappedDown() || (myCall->isWrappedAcross() && myWrapAcrossIsCreatedOnLevel))
			return true;
	}
	else
	if (myCall->isFinalizer())
	{
		return true;
	}

	return false;
}

//=============================
// needsReceival
//=============================
bool CallProperties::needsReceival (void) const
{
	if (myIsOnApplication)
		return false;

	if (myInformationRequired || myCall->isFinalizer())
		return true;

	return false;
}

//=============================
// getNextUniqueId
//=============================
long long CallProperties::getNextUniqueId (void)
{
	long long temp = ourNextUniqueId;
	ourNextUniqueId++;
	return temp;
}

//=============================
// receiveListEqual
//=============================
bool CallProperties::receiveListEqual (std::list<Input*> other)
{
	std::list<Input*>::iterator i,j;

	if (other.size() != myArgsToReceive.size())
		return false;

	for (i = other.begin(); i != other.end(); i++)
	{
		//already in that list ?
		for (j = myArgsToReceive.begin(); j != myArgsToReceive.end(); j++)
		{
			if ((*i)->getName() == (*j)->getName())
				break;
		}

		if (j == myArgsToReceive.end())
		{
			return false;
		}
	}

	return true;
}

//=============================
// usedOrReceiveArgsContain
//=============================
bool CallProperties::usedOrReceiveArgsContain (std::list<Input*> other)
{
    std::list<Input*>::iterator i,j;

    /**
     * Build a super list of all inputs we use in this property, this includes:
     * - myUsedArgs
     * - myArgsToReceive
     * - any input a mapped operation uses
     */
    std::list<Input*> allInputs;
    std::list<Input*>* lists [2] = {&myUsedArgs, &myArgsToReceive};

    for (int listIndex = 0; listIndex < 2; listIndex++)
    {
        std::list<Input*>* listToUse = lists[listIndex];
        for (j = listToUse->begin(); j != listToUse->end(); j++)
            allInputs.push_back(*j);
    }

    std::list<std::pair<Operation*, int > >::iterator opIter;
    for (opIter = myOpsToExecute.begin(); opIter != myOpsToExecute.end(); opIter++)
    {
        Operation* curOp = opIter->first;
        Mapping *m = curOp->getMappingForCall(myCall, opIter->second);
        if (m)
        {
            std::vector<Input*> tInputs = m->getArgumentInputs();
            for (int x = 0; x < tInputs.size(); x++)
                allInputs.push_back(tInputs[x]);
        }
    }

    /**
     * Now compare the given inputs to the super list of all directly and indirectly used inputs
     */
    for (i = other.begin(); i != other.end(); i++)
    {
        bool found = false;

        //already in that list ?
        for (j = allInputs.begin(); j != allInputs.end(); j++)
        {
            //Either both are of the same name (same input) ; Or its an operation input where we don't discern whether we use the array or the array length of the operation (we will have the operation in any case)
            if ((*i)->getName() == (*j)->getName() ||
                    (
                            ((*i)->isArrayInput() || (*j)->isArrayInput()) &&
                            ((*i)->getName() == (*j)->getLenName() || (*i)->getLenName() == (*j)->getName())
                    )
            )
                break;
        }

        if (j == allInputs.end())
        {
            //Alternative is: the input requires an operation and we have this operation in our ops to execute
            Operation* op;
            int id;
            if ((*i)->needsOperation(&op, &id))
            {
                std::list<std::pair<Operation*, int > >::iterator opIter;
                for (opIter = myOpsToExecute.begin(); opIter != myOpsToExecute.end(); opIter++)
                {
                    if (opIter->first == op && opIter->second == id)
                        break;
                }

                if (opIter != myOpsToExecute.end())
                    found = true;
            }
        }
        else
        {
            found = true;
        }

        if (!found)
            return false;
    }//For inputs to check for

    return true;
}

//=============================
// addOperationToExecute
//=============================
void CallProperties::addOperationToExecute (Operation *op, int id)
{
	//check whether this operation and id is already listed
	std::list<std::pair<Operation*, int > >::iterator i;
	for (i = myOpsToExecute.begin(); i != myOpsToExecute.end(); i++)
	{
		if (i->first == op && i->second == id)
			return;
	}

	//add to list of operations
	myOpsToExecute.push_back (std::make_pair(op, id));
}

//=============================
// print
//=============================
std::ostream& CallProperties::print (std::ostream& out) const
{
	std::list<Input*>::const_iterator i;

	if (!myCall)
		return out;

	out
		<< "call=" << myCall->getName() << ", "
		<< "in-uid=" << myInRecordUniqueId << ", "
		<< "on-app=" << myIsOnApplication << ", "
		<< "needs-receival=" << needsReceival() << ", "
		<< "needs-wrapper=" << needsWrapper() << std::endl;

	out
		<< "used-args={";

	for (i = myUsedArgs.begin(); i != myUsedArgs.end(); i++)
	{
		if (i != myUsedArgs.begin())
			out << ", ";
		out << (*i)->getName();
	}

	out
		<< "}" << std::endl
		<< "to-receive-args={";

	for (i = myArgsToReceive.begin(); i != myArgsToReceive.end(); i++)
	{
		if (i != myArgsToReceive.begin())
			out << ", ";
		out << (*i)->getName();
	}

	out
		<< "}" << std::endl
		<< "ops-to-execute={";

	std::list<std::pair<Operation*, int > >::const_iterator opIter;
	for (opIter = myOpsToExecute.begin(); opIter != myOpsToExecute.end(); opIter++)
	{
		if (opIter != myOpsToExecute.begin())
			out << ", ";
		out << opIter->first->getName() << ":" << opIter->second;
	}

	out
		<< "}" << std::endl;

	return out;
}

//=============================
// getMappedOperations
//=============================
std::list<std::pair<Operation*, int > > CallProperties::getMappedOperations (void)
{
	return myOpsToExecute;
}

//=============================
// setWrapAcrossCallAsCreatedOnLevel
//=============================
void CallProperties::setWrapAcrossCallAsCreatedOnLevel (void)
{
    myWrapAcrossIsCreatedOnLevel = true;
}

/*EOF*/
