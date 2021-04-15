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
 * @file Communication.cpp
 * 		@see gti::weaver::Communication
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "Communication.h"

using namespace gti::weaver::layout;

//=============================
// Communication
//=============================
Communication::Communication (void)
 : myCommStrategy (NULL),
   myCommStrategyIntra (NULL),
   myCommProtocol (NULL),
   myStrategySettings (),
   myProtocolSettings ()
{
	/*Nothing to do*/
}

//=============================
// Communication (inter)
//=============================
Communication::Communication (CommStrategy *pStrategy, CommProtocol *pProtocol)
 : myCommStrategy (pStrategy),
   myCommStrategyIntra (NULL),
   myCommProtocol (pProtocol),
   myStrategySettings (),
   myProtocolSettings ()
{
	/*Nothing to do*/
}

//=============================
// Communication (intra)
//=============================
Communication::Communication (CommStrategyIntra *pStrategy, CommProtocol *pProtocol)
 : myCommStrategy (NULL),
   myCommStrategyIntra (pStrategy),
   myCommProtocol (pProtocol),
   myStrategySettings (),
   myProtocolSettings ()
{
    /*Nothing to do*/
}

//=============================
// Communication (inter)
//=============================
Communication::Communication (
		CommStrategy *pStrategy,
		CommProtocol *pProtocol,
		std::list<Setting*> stratSettings,
		std::list<Setting*> protSettings)
 : myCommStrategy (pStrategy),
   myCommStrategyIntra (NULL),
   myCommProtocol (pProtocol),
   myStrategySettings (),
   myProtocolSettings ()
{
	//push setting lists into internal ones
	std::list<Setting*>::iterator i;

	for (i = stratSettings.begin(); i != stratSettings.end(); i++)
		myStrategySettings.push_back (*i);

	for (i = protSettings.begin(); i != protSettings.end(); i++)
		myProtocolSettings.push_back (*i);
}

//=============================
// Communication (intra)
//=============================
Communication::Communication (
        CommStrategyIntra *pStrategy,
        CommProtocol *pProtocol,
        std::list<Setting*> stratSettings,
        std::list<Setting*> protSettings)
 : myCommStrategy (NULL),
   myCommStrategyIntra (pStrategy),
   myCommProtocol (pProtocol),
   myStrategySettings (),
   myProtocolSettings ()
{
    //push setting lists into internal ones
    std::list<Setting*>::iterator i;

    for (i = stratSettings.begin(); i != stratSettings.end(); i++)
        myStrategySettings.push_back (*i);

    for (i = protSettings.begin(); i != protSettings.end(); i++)
        myProtocolSettings.push_back (*i);
}

//=============================
// ~Communication
//=============================
Communication::~Communication (void)
{
	/*
	 * We do not free the strategy and protocol,
	 * their memory is managed by the Gti singleton.
	 */
    myCommStrategy = NULL;
	myCommStrategyIntra = NULL;
	myCommProtocol = NULL;

	//Free the settings
	freeStrategySettings();
	freeProtocolSettings();
}

//=============================
// setCommStrategy
//=============================
void Communication::setCommStrategy ( CommStrategy * new_var )
{
	myCommStrategy = new_var;
	freeStrategySettings ();
}

//=============================
// getCommStrategy
//=============================
CommStrategy* Communication::getCommStrategy (void)
{
	return myCommStrategy;
}

//=============================
// setCommStrategyIntra
//=============================
void Communication::setCommStrategyIntra ( CommStrategyIntra * new_var )
{
    myCommStrategyIntra = new_var;
    freeStrategySettings ();
}

//=============================
// getCommStrategyIntra
//=============================
CommStrategyIntra* Communication::getCommStrategyIntra (void)
{
    return myCommStrategyIntra;
}

//=============================
// setCommProtocol
//=============================
void Communication::setCommProtocol ( CommProtocol * new_var )
{
	myCommProtocol = new_var;
	freeProtocolSettings ();
}

//=============================
// getCommProtocol
//=============================
CommProtocol* Communication::getCommProtocol (void)
{
	return myCommProtocol;
}

//=============================
// addStrategySetting
//=============================
bool Communication::addStrategySetting ( Setting * add_object )
{
	if (!myCommStrategy)
		return false;

	if (!myCommStrategy->isValidSetting(add_object->getName(), add_object->getValue()))
		return false;

	myStrategySettings.push_back (add_object);

	return true;
}

//=============================
// getStrategySettings
//=============================
std::vector<Setting *> Communication::getStrategySettings (void)
{
	return myStrategySettings;
}

//=============================
// addProtocolSetting
//=============================
bool Communication::addProtocolSetting ( Setting * add_object )
{
	if (!myCommProtocol)
		return false;

	if (!myCommProtocol->isValidSetting(add_object->getName(), add_object->getValue()))
		return false;

	myProtocolSettings.push_back (add_object);

	return true;
}

//=============================
// getProtocolSettings
//=============================
std::vector<Setting *> Communication::getProtocolSettings ( void )
{
	return myProtocolSettings;
}

//=============================
// freeStrategySettings
//=============================
void Communication::freeStrategySettings (void)
{
	for (int i = 0; i < myStrategySettings.size(); i++)
	{
		if (myStrategySettings[i])
			delete (myStrategySettings[i]);
	}
	myStrategySettings.clear();
}

//=============================
// freeProtocolSettings
//=============================
void Communication::freeProtocolSettings (void)
{
	for (int i = 0; i < myProtocolSettings.size(); i++)
	{
		if (myProtocolSettings[i])
			delete (myProtocolSettings[i]);
	}
	myProtocolSettings.clear();
}

//=============================
// print
//=============================
std::ostream& Communication::print (std::ostream& out) const
{
	if ((!myCommStrategy && !myCommStrategyIntra) || !myCommProtocol)
	{
		out << "invalidCommunication";
		return out;
	}

	out
		<< "communication={"
		<< "protocol={name=" << myCommProtocol->getModuleName() << ", "
		<< "settings={";

	for (int i = 0; i < myProtocolSettings.size(); i++)
	{
		if (!myProtocolSettings[i])
			continue;

		if (i != 0)
			out << ", ";

		out << *(myProtocolSettings[i]);
	}

	if (myCommStrategy)
        out
            << "}}"
            << ", strategy={name=" << myCommStrategy->getModuleName() << ", "
            << "settings={";
	else
	    out
            << "}}"
            << ", strategyIntra={name=" << myCommStrategyIntra->getModuleName() << ", "
            << "settings={";

	for (int i = 0; i < myStrategySettings.size(); i++)
	{
		if (!myStrategySettings[i])
			continue;

		if (i != 0)
			out << ", ";

		out << *(myStrategySettings[i]);
	}

	out << "}}}";

	return out;
}

/*EOF*/
