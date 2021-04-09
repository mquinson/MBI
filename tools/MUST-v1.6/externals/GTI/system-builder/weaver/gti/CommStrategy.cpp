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
 * @file CommStrategy.cpp
 * 		@see gti::weaver::CommStrategy
 *
 * @author Tobias Hilbrich
 * @date 12.01.2010
 */

#include "CommStrategy.h"

using namespace gti::weaver::modules;

//=============================
// CommStrategy
//=============================
CommStrategy::CommStrategy ( )
	: Configurable ()
{
	/*Nothing to do*/
}

//=============================
// CommStrategy
//=============================
CommStrategy::CommStrategy (
		std::string moduleNameUp,
		std::string configNameUp,
		std::string moduleNameDown,
		std::string configNameDown,
		std::list<SettingsDescription*> settings)
	: Configurable (settings), Module (moduleNameUp,configNameUp),
	  myDownModule (moduleNameDown,configNameDown)
{
	/*Nothing to do*/
}

//=============================
// CommStrategy
//=============================
CommStrategy::CommStrategy (
		std::string moduleNameUp,
		std::string configNameUp,
		std::string moduleNameDown,
		std::string configNameDown,
		std::string instanceType,
		std::string headerName,
		std::string incDir,
		std::list<SettingsDescription*> settings)
	: Configurable (settings),
	  Module (moduleNameUp,configNameUp,instanceType,headerName,incDir),
	  myDownModule (moduleNameDown,configNameDown, instanceType, headerName, incDir)
{
	/*Nothing to do*/
}


//=============================
// CommStrategy
//=============================
CommStrategy::~CommStrategy ( )
{
	/*Nothing to do*/
}

//=============================
// print
//=============================
std::ostream& CommStrategy::print (std::ostream& out) const
{
	out	<< "up-module={";
	Module::print (out);
	out << "}, ";
	out << "down-module={" << myDownModule << "}, ";
	Configurable::print (out);

	return out;
}

//=============================
// getDownModule
//=============================
Module* CommStrategy::getDownModule (void)
{
	return &myDownModule;
}

/*EOF*/
